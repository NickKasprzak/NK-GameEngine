#pragma once
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Common.h"
#include "Coordinator.hpp"
#include "Tilemap.h"

namespace Funny
{
	class Engine
	{
	public:
		static Engine* createInstance()
		{
			if (m_Instance == nullptr)
			{
				m_Instance = new Engine;
				return m_Instance;
			}

			return nullptr;
		}
		static Engine* getInstance() { return m_Instance; }
		static Coordinator* getCoordinator() { return m_Coordinator; }

		bool init(std::string name, int width, int height);
		bool gameLoop();
		bool close();

		static Tilemap* test; // TEMPORARY PLEASE DELETE!!!!!!!!

	private:
		static Engine* m_Instance;
		static Coordinator* m_Coordinator;

		const int IMG_INIT_FLAGS = IMG_INIT_PNG || IMG_INIT_JPG;

		// Temp timestep stuff until a proper physics system is added
		int frame = 0;
		int frameStart = 0;
		float deltaTime;
	};
}