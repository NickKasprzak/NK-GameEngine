#include "Terrain.h"
#include "Window.h"

#include "ImGui/imgui.h"
//#include "ImGui/imgui_impl_win32.h"
#include "ImGui/imgui_impl_sdl2.h"
#include "ImGui/imgui_impl_sdlrenderer2.h"

#include <cmath>

namespace Funny
{
	// We could generate noise based on a heatmap, having certain
	// biomes correspond to certain moistures, generating under
	// different settings based on the biome
	std::vector<std::vector<float>> Terrain::GenerateTerrain()
	{
		std::vector<std::vector<float>> terrain;
		terrain = GenerateWeightedNoiseSum(m_TerrainSettings.frequencies, m_TerrainSettings.weights, m_TerrainSettings.redistribution, m_TerrainSettings.mix, m_TerrainSettings.zoom);
		return terrain;
	}

	std::vector<std::vector<float>> Terrain::GenerateMoisture()
	{
		std::vector<std::vector<float>> moisture;
		moisture = GenerateWeightedNoiseSum(m_MoistureSettings.frequencies, m_MoistureSettings.weights, m_MoistureSettings.redistribution, m_MoistureSettings.zoom);
		return moisture;
	}

	std::vector<std::vector<float>> Terrain::GenerateObjects()
	{
		std::vector<std::vector<float>> objs;
		objs = GenerateWeightedNoiseSum(m_ObjSettings.frequencies, m_ObjSettings.weights, m_ObjSettings.redistribution, 0);
		return objs;
	}

	std::vector<std::vector<float>> Terrain::GenerateWeightedNoiseSum(std::vector<float> frequencies, std::vector<float> weights, float redistribution, float mix, float zoom)
	{
		// Container of noise sum
		std::vector<std::vector<float>> noiseSum;

		// Keeps track of the sum of all
		// added weights, representative
		// of the maximum amplitude range
		// of the noise sum
		float sumOfWeights = 0;

		// Create that sum by looping through
		// all the weights
		for (int i = 0; i < weights.size(); i++)
		{
			sumOfWeights += weights[i];
		}

		// If possible, attempt to reduce this
		// from freq * N^2 time complexity

		// For each pixel in the surface, add the
		// noise value for the given [X, Y] coord
		// to the vector.
		for (int i = 0; i < m_SurfaceSize; i++)
		{
			noiseSum.push_back(std::vector<float>());
			for (int j = 0; j < m_SurfaceSize; j++)
			{
				// Create a sum of noises at this
				// pixel coordinate
				float xyNoiseSum = 0;

				// Get the noise for the current coord
				// at the given frequency and weight
				// pairing of k

				// Values near zero will look funky because
				// the different octaves are generated from the
				// same noise, just scaled differently. RedBlob
				// suggests using different seeds for each octave,
				// but how I'd do that is a little beyond me ATM.
				// Generate a second set of seeds for each octave
				// based on the initial seed?
				// Looks like Simplex doesn't have this problem?
				// Or at least it isnt nearly as visually obvious.
				for (int k = 0; k < frequencies.size(); k++)
				{
					// Update the noise generator to use
					// the current frequency at k
					m_TerrainNoise.SetFrequency((float)frequencies[k]);

					// Get the noise at the current XY coords
					// based on i and j, then scale it by the
					// weight at k. Offset the noise's coord
					// by a power of 2 to sample a different
					// part of the noise for each layer.
					float currentNoise = GetNoise(i, j, zoom);
					currentNoise *= weights[k];

					xyNoiseSum += currentNoise;
				}

				// Scale the noise sum down by the sum
				// of the weights to return it to the
				// 0-1 range.
 				xyNoiseSum /= sumOfWeights;

				// Redistribute the noise sum based on an
				// exponent to push lower values even lower
				xyNoiseSum = pow(xyNoiseSum, redistribution);

				// Island generation

				// Convert the i and j pixel coords to a -1 to 1
				// range based on location relative to the center
				// of the surface. Coords nearing the edges will
				// be closer to -1 or 1 and coords nearing the
				// center will be closer to 0.
				float x = ((2.0f * i) / (m_SurfaceSize)) - 1.0f;
				float y = ((2.0f * j) / (m_SurfaceSize)) - 1.0f;

				// Use the euclidian squared distance function
				// to calculate a circular distance to the center
				// of the surface based on the previously calculated
				// x and y coordinates.
				float dist = fmin(1, ((pow(x, 2) + pow(y, 2)) / sqrt(2.0f)));

				// Linearly interpolate between our current noise
				// value and the distance to the center of the
				// surface by a factor of mix, pushing the noise
				// to be more circular depending on how much
				// the distance to the center should mix into the
				// final noise value.

				// At higher mixes, the distance will factor into
				// changing the noise value more, so the sum will
				// be interpolated more strongly by its distance
				// to the center, resulting in higher elevations
				// closer to the center.
				xyNoiseSum = xyNoiseSum + (mix * ((1.0 - dist) - xyNoiseSum));

				// Push the value into the noise sum
				// vector.
				noiseSum[i].push_back(xyNoiseSum);
			}
		}

		return noiseSum;
	}

	float Terrain::GetNoise(int x, int y, float zoom)
	{
		// Convert the given Noise value (ranged -1 to 1)
		// into the 0-1 range.

		// Scales the given x and y coordinate down by the
		// size of the target surface so the generate noise
		// remains the same regardless of the quality of the
		// output.
		//
		// I assume this is because the coordinates given to
		// the noise generator range from 0-1, ranging as one
		// full wavelength. Not scaling the value down will
		// result in advancing across generation too quickly.
		//
		// Doing this also lets us represent the same noise
		// generation input the same regardless of the quality.
		// A 300x300 screen will represent noise the same way
		// as a 1000x1000 screen. Showing different values
		// is as simple as sampling a larger radius.
		//
		// Zooming in and out is as simple as scaling the given
		// x and y value by something. Powers of positive 10 zoom
		// out and powers of negative 10 zoom in.
		//
		// That info is a little useless though since zooming
		// in and out should be done independently of a texture's
		// resolution. Our renderer should handle that stuff for
		// me if I were to apply this to a world setting with
		// a moveable player camera.
		//
		// But what if I want to sample a larger area of noise?
		// Then I could just pass in a noise area on terrain
		// generation so the given values would still show
		// a larger area, just in a smaller resolution.
		float noiseVal = m_TerrainNoise.GetNoise((float)(x * zoom) / (m_SurfaceSize), (float)(y * zoom) / (m_SurfaceSize));

		noiseVal += 1;
		noiseVal /= 2;

		return noiseVal;
	}

	// Generates a set of frequencies and weights based on the given
	// values and stores the generated info in the terrain settings struct
	void Terrain::GenerateTerrainSettings(int frequencyRange, float amplitudeFrac, float redistribution, float mix, float zoom)
	{
		float amplitude = 1;

		m_TerrainSettings.frequencies.clear();
		m_TerrainSettings.weights.clear();

		for (int i = 1; i < frequencyRange + 1; i++)
		{
			m_TerrainSettings.frequencies.push_back((float)i);
			m_TerrainSettings.weights.push_back(amplitude);
			amplitude /= amplitudeFrac;
		}

		m_TerrainSettings.redistribution = redistribution;
		m_TerrainSettings.mix = mix;
		m_TerrainSettings.zoom = zoom;
	}

	// Same deal as with GenerateTerrainSettings, except it generates
	// values for the moisture settings struct
	void Terrain::GenerateMoistureSettings(int frequencyRange, float amplitudeFrac, float redistribution, float waterLevel, float zoom)
	{
		float amplitude = 1;

		m_MoistureSettings.frequencies.clear();
		m_MoistureSettings.weights.clear();

		for (int i = 1; i < frequencyRange + 1; i++)
		{
			m_MoistureSettings.frequencies.push_back((float)i);
			m_MoistureSettings.weights.push_back(amplitude);
			amplitude /= amplitudeFrac;
		}

		m_MoistureSettings.redistribution = redistribution;
		m_WaterLevel = waterLevel;
		m_MoistureSettings.zoom = zoom;
	}

	void Terrain::GenerateObjectGenSettings(int frequencyRange, float amplitudeFrac, float redistribution, float zoom)
	{
		float amplitude = 1;

		m_ObjSettings.frequencies.clear();
		m_ObjSettings.weights.clear();

		for (int i = 1; i < frequencyRange + 1; i++)
		{
			m_ObjSettings.frequencies.push_back((float)i);
			m_ObjSettings.weights.push_back(amplitude);
			amplitude /= amplitudeFrac;
		}

		m_ObjSettings.redistribution = redistribution;
		m_ObjSettings.zoom = zoom;
	}

	// Naive biomes
	// A more complex and robust solution
	// is too out of scope for now.
	// Still, really unhappy with this.
	// 
	// Generates a color representing
	// either biome or a tree (RGBA of 20, 50, 0, 255)
	ColorRGBA Terrain::GetBiome(float terrainHeight, float moistureLevel, float blueNoise)
	{
		if (terrainHeight < m_WaterLevel)
		{
			return ColorRGBA(70, 70, 255, 255);
		}

		else if (terrainHeight < 0.4)
		{
			if (moistureLevel < 0.3)
			{
				return ColorRGBA(66, 66, 66, 255);
			}

			else if (moistureLevel < 0.6)
			{
				return ColorRGBA(150, 150, 110, 255);
			}

			else
			{
				return ColorRGBA(240, 220, 150, 255);
			}
		}

		else if (terrainHeight < 0.8)
		{
			if (moistureLevel < 0.3)
			{
				if (blueNoise > 0.9)
				{
					return ColorRGBA(20, 50, 0, 255);
				}
				return ColorRGBA(60, 80, 40, 255);
			}

			else if (moistureLevel < 0.6)
			{
				if (blueNoise > 0.6)
				{
					return ColorRGBA(20, 50, 0, 255);
				}
				return ColorRGBA(110, 80, 70, 255);
			}

			else
			{
				if (blueNoise > 0.4)
				{
					return ColorRGBA(20, 50, 0, 255);
				}
				return ColorRGBA(70, 200, 0, 255);
			}
		}

		else
		{
			if (moistureLevel < 0.5)
			{
				return ColorRGBA(70, 70, 70, 255);
			}

			else
			{
				if (blueNoise > 0.95)
				{
					return ColorRGBA(20, 50, 0, 255);
				}
				return ColorRGBA(255, 255, 255, 255);
			}
		}
	}

	void Terrain::UpdateSurfaceDisplay()
	{
		std::vector<std::vector<float>> terrain = GenerateTerrain();
		std::vector<std::vector<float>> moisture = GenerateMoisture();
		std::vector<std::vector<float>> objNoise = GenerateObjects();

		for (int i = 0; i < m_SurfaceSize; i++)
		{
			for (int j = 0; j < m_SurfaceSize; j++)
			{
				// Terrain and biome gen
				float terrainVal = terrain[i][j];
				float moistureVal = moisture[i][j];
				float objVal = objNoise[i][j];

				ColorRGBA biomeColor = GetBiome(terrainVal, moistureVal, objVal);
				DrawPixel(i, j, biomeColor);
			}
		}
	}

	// Unsafe(?) returning an allocation, replace with method
	// that moves generated texture to the provided pointer instead.
	SDL_Texture* Terrain::ExportToTexture()
	{
		UpdateSurfaceDisplay();
		SDL_Texture* newTexture = SDL_CreateTextureFromSurface(Window::getSDLRenderer(), m_Surface);
		return newTexture;
	}

	void Terrain::DrawPixel(int x, int y, ColorRGBA color)
	{
		// Get number of pixels that need to jumped over
		// from pixel data to get to the target pixel.
		// pitch is the number of bytes per row (y).
		int memOffset = (x * m_Surface->format->BytesPerPixel) + (y * m_Surface->pitch);
		char* pixelTarget = (char*)m_Surface->pixels;

		// Set red
		*(pixelTarget + memOffset) = color.r;

		// Set green
		*(pixelTarget + memOffset + 1) = color.g;

		// Set blue
		*(pixelTarget + memOffset + 2) = color.b;

		// Set alpha
		*(pixelTarget + memOffset + 3) = color.a;
	}

	// wont work properly, just moved here to free up main.cpp
	void Terrain::ImGuiDebug()
	{
		// ImGui initialization
		// 
		// Creates a new ImGui context and
		// configures it to accept input
		// and have dark theme.
		// 
		// Then, our SDL2 window and renderer
		// is given to it so it can render its
		// GUI stuff through SDL2's rendering
		// system.

		// move this to engine/system init
		// if you plan on using imgui.
		// or just abstract this stuff better
		// in the first place
		{
			IMGUI_CHECKVERSION();
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
			io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
			ImGui::StyleColorsDark();
			ImGui_ImplSDL2_InitForSDLRenderer(Funny::Window::getSDLWindow(), Funny::Window::getSDLRenderer());
			ImGui_ImplSDLRenderer2_Init(Funny::Window::getSDLRenderer());
		}

		// ImGui value storage
		int imguiTerrainFreqRange = 1;
		float imguiTerrainAmplitudeScale = 1;
		float imguiTerrainRedistribution = 1;
		float imguiTerrainMix = 0;

		int imguiMoistureFreqRange = 1;
		float imguiMoistureAmplitudeScale = 1;
		float imguiMoistureRedistribution = 1;

		float imguiWaterLevel = 0.3;

		// It'd probably be good to shift the ImGui stuff to
		// a different class (ie. Engine, Window, System)
		// so its startup is handled elsewhere

		// It'd also probably be good to move the terrain
		// specific ImGui stuff to the terrain class header
		// so I can toggle it for debugging and testing
		// some other time

		// Maybe have some kind of array of function pointers
		// in the window class that runs through a series
		// of functions that handle ImGui operations

		// Create basic ImGui window for terrain
		{
			ImGui::Begin("Terrain Settings");

			ImGui::InputInt("Frequency Range", &imguiTerrainFreqRange, 1, 1);
			ImGui::InputFloat("Amplitude Weight", &imguiTerrainAmplitudeScale, 0.1, 1);
			ImGui::InputFloat("Redistribution", &imguiTerrainRedistribution, 0.1, 1);
			ImGui::InputFloat("Mix", &imguiTerrainMix, 0.01, 0.1);

			if (ImGui::Button("Generate Terrain"))
			{
				//SDL_DestroyTexture(terrainTexture);

				GenerateTerrainSettings(imguiTerrainFreqRange, imguiTerrainAmplitudeScale, imguiTerrainRedistribution, imguiTerrainMix, 4);
				//terrainTexture = testTerrain.ExportToTexture();
			}

			ImGui::End();
		}

		// Create basic ImGui window for 
		{
			ImGui::Begin("Biome Settings");

			ImGui::Text("Moisture Map");
			ImGui::InputInt("Frequency Range", &imguiMoistureFreqRange, 1, 1);
			ImGui::InputFloat("Amplitude Weight", &imguiMoistureAmplitudeScale, 0.1, 1);
			ImGui::InputFloat("Redistribution", &imguiMoistureRedistribution, 0.1, 1);

			ImGui::Text("Misc");
			ImGui::InputFloat("Water Level", &imguiWaterLevel, 0.1, 1);

			if (ImGui::Button("Generate Biomes"))
			{
				//SDL_DestroyTexture(terrainTexture);

				GenerateMoistureSettings(imguiMoistureFreqRange, imguiMoistureAmplitudeScale, imguiMoistureRedistribution, imguiWaterLevel, 0.5);
				//terrainTexture = testTerrain.ExportToTexture();
			}

			ImGui::End();
		}

		// Render created ImGui stuff to the
		// current ImGui frame
		ImGui::Render();
	}
}