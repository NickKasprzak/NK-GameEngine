#pragma once
#include <string>
#include <iostream>

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"

namespace ImageOLD
{
	// Store raw surface image data here.
	// Pass raw pixel data from the surface to a variable
	// Keep track of width, height, and size of raw data

	// When it comes to drawing, pass relevant data to a renderer
	// Renderer should accept void* for pixel data among other things
	// Have renderer generate temp Texture object using raw data
	// Have renderer draw texture data to the screen at the desired location
	// Renderer should delete the texture after use

	// Renderer should hold an SDL window and renderer
	// Images should be loaded to and save in memory at an accessible location like a manager for ease of reuse
	// Above should also save on memory
	// 
	// Because images can vary greatly in size, how should I go about saving them in memory? Its not as a simple as just creating
	// an object pool or something because that depends on its objects being similar in size. 
	// 
	// Would I have to create a stack allocator? I could push image data onto the stack, keeping track of pointers to the start
	// of their respective image data, as well as being able to iterate through them one at a time by adding the image's size to
	// their location in memory. I could also use this to flag blocks of memory as open and realign memory by shifting things
	// around to be closer to each other (memcpy image data from current location to end of previously allocated block), avoiding
	// memory fragmentation in the process. Only issue is that making insertions and deletions would end up being a costly process
	// and the preallocation of a block of memory could be wasteful if we aren't working with large images.
	// 
	// Theres also the problem of aligning the other data bundled with the pixel data
	// 
	// Check out how OGRE does it and take notes (if you can find how they do it at all)
	// Looks like they just use their own macro for new thats been templated based on type just for the raw image data
	// I assume its ok since these allocations dont happen as often as something like component creation and deletion,
	// as these images are usually only loaded to memory once at startup and only get reused later on to generate textures
	// and such at runtime, so there shouldnt be too much concern about memory fragmentation. Plus, anything that relies on
	// rapid creation and deletion should have their own aligned pools of memory to work with anyways.
	// 
	// Also look into shared pointers, might be helpful for managing multiple references to one object across many (ie. Particle)

	// Might not be needed?
	struct ImageFormatData_OLD
	{
		// Values corresponding to the bits we need to
		// flip using AND (&) to isolate their respective
		// values in their encoded format
		Uint32 rMask = 0;
		Uint32 gMask = 0;
		Uint32 bMask = 0;
		Uint32 aMask = 0;

		// How many bits we need to shift the isolated
		// value from where its stored to be located at
		// the start of a given location in memory
		Uint8 rShift = 0;
		Uint8 gShift = 0;
		Uint8 bShift = 0;
		Uint8 aShift = 0;

		// How many bits lost when compacting the given
		// value to an 8-bit format, being the amount of
		// bits we have to expand/shift-right the value
		// by so its a full 8-bit value
		Uint8 rLoss = 0;
		Uint8 gLoss = 0;
		Uint8 bLoss = 0;
		Uint8 aLoss = 0;
	};

	struct ImageData_OLD
	{
		char* imgData = nullptr; // image is stored at the byte level
		Uint32 imgWidth = 0;
		Uint32 imgHeight = 0;
		Uint32 imgPitch = 0; // pitch is the amount of bytes per row.
		// needs to be specified as sometimes images are allocated with some bytes of padding after each row
		// this means that width * height * bytesperpixel could be incorrect for calculating image byte size
		Uint32 imgBitsPerPixel = 0;
		Uint32 imgBytesPerPixel = 0;
		size_t imgSize = 0;
		ImageFormatData_OLD imgFormatData;
	};

	class Image_OLD
	{
	public:
		bool loadImage(std::string path)
		{
			// Could be interesting to load image data manually without SDL2. Its just streaming stuff from a file right?
			// I could also learn how image metadata(?) gets stored and read in too.
			SDL_Surface* tempSurface = IMG_Load(path.c_str());
			SDL_LockSurface(tempSurface);

			if (tempSurface == nullptr)
			{
				std::cout << "Failed to load image at path " << path << "." << std::endl;
				return false;
			}

			if (tempSurface->format->BitsPerPixel == 8)
			{
				std::cout << "8-bit image formats are unsupported." << std::endl;
				return false;
			}

			// Copy data from SDL surface to ImageData struct
			m_Data.imgWidth = tempSurface->w;
			m_Data.imgHeight = tempSurface->h;
			m_Data.imgPitch = tempSurface->pitch;
			m_Data.imgBitsPerPixel = tempSurface->format->BitsPerPixel;
			m_Data.imgBytesPerPixel = tempSurface->format->BytesPerPixel;
			m_Data.imgSize = m_Data.imgPitch * m_Data.imgHeight;

			ImageFormatData_OLD format;
			format.rMask = tempSurface->format->Rmask;
			format.gMask = tempSurface->format->Gmask;
			format.bMask = tempSurface->format->Bmask;
			format.aMask = tempSurface->format->Amask;

			format.rShift = tempSurface->format->Rshift;
			format.gShift = tempSurface->format->Gshift;
			format.bShift = tempSurface->format->Bshift;
			format.aShift = tempSurface->format->Ashift;

			format.rLoss = tempSurface->format->Rloss;
			format.gLoss = tempSurface->format->Gloss;
			format.bLoss = tempSurface->format->Bloss;
			format.aLoss = tempSurface->format->Aloss;
			m_Data.imgFormatData = format;

			// Reserve required space in bytes for the image based on
			// total pixel count and the bytes per pixel
			m_Data.imgData = new char[m_Data.imgSize];
			// Move pixel data to reserved memory
			memcpy(m_Data.imgData, tempSurface->pixels, m_Data.imgSize);

			std::cout << "Successfully loaded image at path " << path << "." << std::endl;
			std::cout << "Width: " << m_Data.imgWidth << std::endl;
			std::cout << "Height: " << m_Data.imgHeight << std::endl;
			std::cout << "Pitch: " << m_Data.imgPitch << std::endl;
			std::cout << "Bits per pixel: " << m_Data.imgBitsPerPixel << std::endl;
			std::cout << "Bytes per pixel: " << m_Data.imgBytesPerPixel << std::endl;
			std::cout << "Total size: " << m_Data.imgSize << std::endl;

			SDL_UnlockSurface(tempSurface);
			SDL_FreeSurface(tempSurface);

			return true;
		}

		// No need to scale pixel number by byte count per pixel. Uint32 pointer does that for us (4 bytes per pixel, 32 bits per pixel)
		// Only need to scale by bytes if the image is stored as bits, which is probably the better option anyways for different image formats.
		// We can use char, Uint32, and other bit-measured data types to read in the specified bit count from a given space in memory as one value
		// char is a byte

		// Not sure why but indexing to pixel RGBA values is completely different than whats listed on the SDL2 website and theres no need for
		// any of the mask, shift, and loss stuff. If I had to guess, maybe its just a result of the image format I loaded or moving the data out
		// of the SDL Surface with memcpy, but the values are arranged in such a way that I can just add 1 to the offset for G and 2 for B. I think
		// this implies that our RGBA values are just lined up byte by byte, each pixel consisting of four bytes, one for each color component.
		// This could very well break with other non-32 bit RGBA image formats though, but it works fine for now?

		// Big understanding breakthrough!!!!! We jump between spots in memory based on the amount of bits our pointer is equal to. If we have a char
		// pointer, then adding to a pointer will advance the pointer by however much was added in chars, or 1 byte. Another example, if we have a
		// Uint32 pointer, adding to that pointer will advance it 32 bits at a time. This also applied to writing to that space in memory too, as
		// writing to any pointer in memory will only write however many bits are specified by the pointer. If a pointer is a char, we only write 4
		// bytes in size from that point. If a pointer is a Uint32, we only write 8 bytes in size from that point.

		// This means that if we cast our pixel data, stored as a char/byte, to a Uint32 pointer, then any writes will cover 4 bytes. This means that
		// writing a Uint32 at a time will write to all of our RGBA pixel components rather than just the individual components.

		// Im guessing if we attempt to write a Uint32 value at a char address, the 8 byte value gets truncated down to a byte in order to fit.

		// There are so many formats holy shit, no way am I wrapping all of that lol
		void readPixel(int x, int y)
		{
			int offset = (x * m_Data.imgBytesPerPixel) + (y * m_Data.imgPitch);

			Uint8 redColorVal = 0;
			redColorVal = *(m_Data.imgData + offset);

			Uint8 greenColorVal = 0;
			greenColorVal = *(m_Data.imgData + offset + 1);

			Uint8 blueColorVal = 0;
			blueColorVal = *(m_Data.imgData + offset + 2);
			std::cout << "Pixel color at X: " << x << " Y: " << y << " is ["
				<< (int)redColorVal << ", "
				<< (int)greenColorVal << ", "
				<< (int)blueColorVal << "]." << std::endl;
		}

		void writePixel(int x, int y, Uint32 red, Uint32 green, Uint32 blue)
		{
			int offset = (x * m_Data.imgBytesPerPixel) + (y * m_Data.imgPitch);

			char* redColorRef = (m_Data.imgData);
			*redColorRef = red;

			char* greenColorRef = (m_Data.imgData + 1);
			*greenColorRef = green;

			char* blueColorRef = (m_Data.imgData + 2);
			*blueColorRef = blue;
		}

		ImageData_OLD getImageData() { return m_Data; }

	private:
		ImageData_OLD m_Data;
	};
}