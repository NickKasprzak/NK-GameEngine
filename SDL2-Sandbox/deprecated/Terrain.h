#pragma once
#include <vector>
#include "FastNoiseLite.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include "Color.h"

// try object placement with blue noise that might be cool
// each biome could base their tree distribution based on
// how low/high of an amplitude the generate blue noise
// has to be in order to place a tree. low amplitudes means
// more trees since that means a lower bar for the blue
// noise to pass in order to place a tree, while higher
// amplitudes are the opposite.
namespace Funny
{
	struct TerrainGenSettings
	{
		std::vector<float> frequencies;
		std::vector<float> weights;
		float redistribution = 1;
		float mix = 0;
		float zoom = 1;
	};

	class Terrain
	{
	public:
		Terrain(int size)
			: m_SurfaceSize(size)
		{
			m_TerrainNoise.SetSeed(2343240);
			m_TerrainNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
			m_Surface = SDL_CreateRGBSurface(0, size, size, 32, 0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);

			GenerateObjectGenSettings(50, 0.175f, 3, 4);
		}

		~Terrain()
		{
			SDL_FreeSurface(m_Surface);
		}

		void ImGuiDebug();

		// Generates a series of weighted noise maps to overlay
		// and create things like a heightmap or object layout
		std::vector<std::vector<float>> GenerateTerrain();
		std::vector<std::vector<float>> GenerateMoisture();
		std::vector<std::vector<float>> GenerateObjects();

		void GenerateTerrainSettings(int frequencyRange, float amplitudeFrac, float redistribution, float mix, float zoom);
		void GenerateMoistureSettings(int frequencyRange, float amplitudeFrac, float redistribution, float waterLevel, float zoom);
		void GenerateObjectGenSettings(int frequencyRange, float amplitudeFrac, float redistribution, float zoom);

		ColorRGBA GetBiome(float terrainHeight, float moistureLevel, float blueNoise);

		void UpdateSurfaceDisplay();

		// cant resize a surface without creating a new one or jumping through some hoops.
		// would probably be easier to just allocate a large surface and then handle drawing
		// scaled at different resolutions.
		// 
		// that solution actually kinda sucks. it makes EVERY update surface draw call exceedingly
		// slow regardless of the resolution of the noise. recreating the surface is only an issue
		// if it ends up fragmenting the heap, which I dont know if it does since SDL handles it.
		// from a glance it looks like SDL does all of its own memory handling, only allocating
		// and deallocating from its own defined spaces in memory, so i probably dont need to
		// worry about that stuff?
		//void UpdateSurfaceSize(int x, int y);
											
		SDL_Texture* ExportToTexture();

	private:
		// Consider multiple noise generators for different purposes
		// Stuff like basic terrain, octaves added for roughness,
		// heat maps for differnt biomes, etc.
		//
		// Would it be possible to only have certain types of noise
		// generate within certain heat ranges?
		FastNoiseLite m_TerrainNoise;
		SDL_Surface* m_Surface = nullptr;
		int m_SurfaceSize = 0;
		float m_Zoom = 4;
		float m_WaterLevel = 0.3f;

		void DrawPixel(int x, int y, ColorRGBA color);

		TerrainGenSettings m_TerrainSettings;
		TerrainGenSettings m_MoistureSettings;
		TerrainGenSettings m_ObjSettings;

		// Create a weighted sum of noises based on the given frequencies
		// and corresponding weights.
		//
		// Helper function used in the terrain generation process to create
		// weighted noise maps for things like pink noise for terrain or
		// blue noise for object placement. Or a heat map for biomes
		//
		// This should be used only to generate certain colors of noise
		// based on frequencies and weights. Any other types that would
		// require different generation (like octaves for roughness)
		// should be done through other functions.
		std::vector<std::vector<float>> GenerateWeightedNoiseSum(std::vector<float> frequencies, std::vector<float> weights, float redistribution, float mix, float zoom = 1);

		// Helper function to convert noise at given coords to
		// a 0-1 range.
		float GetNoise(int x, int y, float zoom);
	};
}