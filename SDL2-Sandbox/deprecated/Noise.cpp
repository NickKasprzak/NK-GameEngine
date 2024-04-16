#include "Noise.h"
#include "Window.h"

namespace Funny
{
	SDL_Surface* NoiseGenerator::GenerateNoiseSurface(int size)
	{
		// Create noise surface with a 32 bit format,
		// with each color being ordered in RGBA byte order,
		// large enough to fit the size on both dimensions
		// Might vary depending on device byte endianess
		// (RGBA vs ABGR), so pixel read/writes would need
		// to be written differently through a compiler
		// macro to shift around what bytes it writes to
		noiseSurface = SDL_CreateRGBSurface(0, size, size, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
		
		std::vector<std::vector<float>> noise;
		noise.reserve(size);
		noise = CreateWaveSum2D(10, -1, size);
		FillSurface(noise, noiseSurface);
		
		return noiseSurface;
	}

	std::vector<std::vector<float>> NoiseGenerator::CreateWaveSum2D(float frequencyMax, int exponent, int size)
	{
		// Generate a bunch of 2D sine waves for
		// the specified range of frequencies,
		// adding them all to a sum
		std::vector<std::vector<float>> waveSum;
		float amplitudeSum = 0;

		// fill wave sum with empty values
		for (int i = 0; i < size; i++)
		{
			waveSum.push_back(std::vector<float>());
			for (int j = 0; j < size; j++)
			{
				waveSum[i].push_back(0);
			}
		}

		// create a sum of all wave values
		for (int i = 0; i < frequencyMax; i++)
		{
			std::vector<std::vector<float>> currentWave;
			float amplitude = pow(i + 1, exponent);
			amplitudeSum += amplitude;
			currentWave = CreateSineWave2D(i + 1, amplitude, size);

			for (int j = 0; j < size; j++)
			{
				for (int k = 0; k < size; k++)
				{
					waveSum[j][k] += currentWave[j][k];
				}
			}
		}

		// Get highest possible amplitude range
		// Positive exponents: pow(freqmax, exponent)
		// 0 or negative exponents: 1
		//float maxAmplitude = pow(frequencyMax, exponent);

		// average all the wave values
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < size; j++)
			{
				
			}
		}

		return waveSum;
	}

	// This is super unoptimal and naive. Try to find a faster implementation.
	// Honestly if I were to try to apply something like this to a tilegrid,
	// itd probably be faster to have terrain be generated in a low resolution
	// and then tiles could be smoothed between the heights, similarly to a
	// vertex/edge pairing. Linearly interpolating between two points, slapping
	// down a tile a location based on the distance between two heightmap locations?
	// I think lerping would also make it easier to create continuous noise (like
	// infinite terrain or something) but I have no clue how that works at a
	// deeper level.
	std::vector<std::vector<float>> NoiseGenerator::CreateSineWave2D(int frequency, float amplitude, int size)
	{
		// Generate two samples of 1D noise
		std::vector<float> waveRowVertical;
		waveRowVertical.reserve(size);
		CreateSineWave1D(frequency, amplitude, waveRowVertical);

		std::vector<float> waveRowHorizontal;
		waveRowHorizontal.reserve(size);
		CreateSineWave1D(frequency, amplitude, waveRowHorizontal);

		// Apply this noise across a full vector vertically
		std::vector<std::vector<float>> verticalNoise;
		for (int i = 0; i < size; i++)
		{
			verticalNoise.push_back(waveRowVertical);
		}

		
		// Apply this noise again across a full vector horizontally
		std::vector<std::vector<float>> horizontalNoise;
		for (int i = 0; i < size; i++)
		{
			horizontalNoise.push_back(std::vector<float>());
			for (int j = 0; j < size; j++)
			{
				horizontalNoise[i].push_back(waveRowHorizontal[i]);
			}
		}

		std::vector<std::vector<float>> averageNoise;
		for (int i = 0; i < size; i++)
		{
			averageNoise.push_back(std::vector<float>());
			for (int j = 0; j < size; j++)
			{
				averageNoise[i].push_back((horizontalNoise[i][j] * verticalNoise[i][j])); // dot? div by 2?
			}
		}

		return averageNoise;
	}

	// Ideally we want to do the noise normalization somewhere between when the
	// final weighted sum is produced and here. This is so that the normalization
	// is done relative to the maximum/minimum range of the entire sum rather
	// than the individual waves amplitudes, since adding a bunch of waves that
	// have been normalized to 0-1 will just result in a bunch of waves with
	// the same weights.

	// That said, we could just average all the waves to the maximum amplitude
	// range so everything is normalized to that maximum peak before generating
	// the sum. Only issue there (besides being wrong) is floating point precision.
	// Dividing an already small float by something like 30 would result in a
	// number thats too small to be represented more often than not. Bytes shouldn't
	// be a concern since floats are always 4 bytes.

	// But what about combining noise? Sure we could handle things beyond this
	// point via weight maps within the context of the product (thing only happens
	// in this value range) but that would be easier to apply within a 0-1 range
	// and it could be applied to other mediums easier too(?)
	void NoiseGenerator::FillSurface(std::vector<std::vector<float>>& noiseData, SDL_Surface* surface)
	{
		for (int i = 0; i < noiseData.size(); i++)
		{
			for (int j = 0; j < noiseData[i].size(); j++)
			{
				float noise = noiseData[i][j];
				
				float noiseRGBA = noise * 255.0f;

				SetPixelColor(i, j, noiseRGBA, noiseRGBA, noiseRGBA, 255);
			}
		}
	}

	// Would it be possible to just memcpy all of the color data to the pixel buffer?
	void NoiseGenerator::SetPixelColor(int x, int y, Uint32 red, Uint32 green, Uint32 blue, Uint32 alpha)
	{
		// Get number of pixels that need to jumped over
		// from pixel data to get to the target pixel.
		// pitch is the number of bytes per row (y).
		int memOffset = (x * noiseSurface->format->BytesPerPixel) + (y * noiseSurface->pitch);
		char* pixelTarget = (char*)noiseSurface->pixels;

		// Set red
		*(pixelTarget + memOffset) = red;

		// Set green
		*(pixelTarget + memOffset + 1) = green;

		// Set blue
		*(pixelTarget + memOffset + 2) = blue;

		// Set alpha
		*(pixelTarget + memOffset + 3) = alpha;
	}
}