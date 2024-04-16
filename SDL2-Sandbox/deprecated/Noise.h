#pragma once
#include <random>
#include <math.h>
#include <vector>
#include <SDL2/SDL.h>
#include "Vector.h"

namespace Funny
{
	const int WAVE_PERIOD_INT = 2 * 314;
	const float WAVE_PERIOD = 2 * 3.14;

	// lol turns out this was using up 160000 bytes. too much for stack
	/*
	struct NoiseMap
	{
		const static int MAX_WIDTH = 200;
		const static int MAX_HEIGHT = 200;
		float map[MAX_WIDTH][MAX_HEIGHT];
	};
	*/

	/*
	* Our noise generator needs to take in a
	* set of frequencies (can be a range or
	* set of octaves) and a power that these
	* frequencies' corresponding amplitudes
	* should correspond to.
	* 
	* The function should also be given a range
	* of values on the x and y axis corresponding
	* to how large of a chunk it should generate
	* noise for. This can either be through a
	* reference to a pre-allocated list/vector
	* or specific arguments used when generating
	* an array to return.
	*/

	// also little afraid of using a vector. what does it mean for performance?
	class NoiseGenerator
	{
	public:
		// IDK if this is how infinite generation works,
		// but it feels like a sine-based noise generation
		// has to keep track of the waves being used by the
		// different frequency layers.

		// either that or theres some kind of interpolation
		// between each section 

		// perlin and simplex use gradients, so I assume thats
		// how chunks of noise generation can smoothly transition
		// between each other?
		void InitRandOffset()
		{
			// do nothing
		}

		void ClearSurface()
		{
			SDL_FreeSurface(noiseSurface);
			return;
		}

		std::vector<float> CreateWaveSum1D(float frequencyMax, int exponent, int chunkSize, int chunkOffset)
		{
			std::vector<std::vector<float>> noises;
			std::vector<float> noise_sum;

			// Generate waves at all the specified frequencies
			for (int i = 1; i < frequencyMax; i++)
			{
				// Get and resize noise vector to hold all needed values.
				std::vector<float> currentNoiseVec;
				currentNoiseVec.reserve(chunkSize);

				// Calculate the amplitude using the exponent provided
				// and the current frequency of i. Depending on how the
				// exponent is oriented (+/-), it should generate a
				// frequency spectrum that is positively or negatively
				// inclined, making certain frequencies more prominent
				// in the weighted sum due to having higher or lower
				// amplitudes.
				float amplitude = pow(i, exponent);

				// Generate a sine wave using the current frequency
				// and amplitude, passing it to the current noise vector
				CreateSineWave1D(i, amplitude, currentNoiseVec);

				noises.push_back(currentNoiseVec);
			}

			// Generate a weighted sum by averaging all the waves
			for (int i = 0; i < chunkSize; i++)
			{
				float currentSum = 0;

				for (int j = 0; j < frequencyMax - 1; j++)
				{
					currentSum += noises[j][i];
				}

				// do any scaling here
				noise_sum.push_back(currentSum);
			}

			return noise_sum;
		}

		// Creating this for 2D is just applying this process
		// to a 2D space on an X and Y axis, then combining them
		// as one 2D wave by taking their sum?

		// I could return a 2D wave from this, both being generated
		// using the same shift and amplitude, and then make noise
		// using their sums if I'm understanding this properly.
		void CreateSineWave1D(int frequency, float amplitude, std::vector<float>& waveArray)
		{
			// Pick a random phase shift ranging
			// between 0 and 2pi, the length of
			// a wave's period.
			int noiseResolution = waveArray.capacity();
			float shift = (float)(rand() % WAVE_PERIOD_INT) / 100.0f;

			// Generate wave heights based on the
			// given frequency for the total size
			// of the wave thats offset by the
			// random phase
			for (int i = 0; i < waveArray.capacity(); i++)
			{
				// 2pi is the basic length of the period
				// 
				// We scale that by our frequency so our
				// period is adjusted to match how frequent
				// our wave occurs
				//
				// That then scaled by i to get the wave's
				// height at the x position corresponding to
				// the value of i
				//
				// We then divide that by the length of the
				// array, or the map's size, to scale it
				// to be relative to the size of our map
				//
				// Lastly, we add our phase shift to the
				// value so the wave is offset randomly.
				float noiseVal = sin((WAVE_PERIOD * (float)frequency * (float)(i + noiseResolution) / (float)waveArray.capacity() + shift));

				// By scaling the wave by the map length,
				// we're still producing the same wavelength
				// regardless of the map's size, only now
				// we're adjusting the resolution to fit
				// how many values it can be represented by.
				// If we want our wave to be clearer in lower
				// resolutions, the points representing it
				// would have to be spread out more.

				// Lastly, we scale the wave by the amplitude,
				// adjusting its overall strength.
				noiseVal = amplitude * noiseVal;

				// temporarily normalize
				//noiseVal = (noiseVal + 1.0f) / 2.0f;

				waveArray.push_back(noiseVal);
			}
		}

		SDL_Surface* GenerateNoiseSurface(int size);
		std::vector<std::vector<float>> CreateWaveSum2D(float frequencyMax, int exponent, int size);
		std::vector<std::vector<float>> CreateSineWave2D(int frequency, float amplitude, int size);

	private:
		SDL_Surface* noiseSurface = nullptr;
		int noiseSize = 0;
		void FillSurface(std::vector<std::vector<float>>& noiseData, SDL_Surface* surface);
;		void SetPixelColor(int x, int y, Uint32 red, Uint32 green, Uint32 blue, Uint32 alpha);
	};
}