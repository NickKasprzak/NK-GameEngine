#include "Engine.h"
#include "RenderSystem.h"
#include "ResourceManager.h"

namespace Funny
{
	Engine* Engine::m_Instance = 0;
	Coordinator* Engine::m_Coordinator = 0;
	Tilemap* Engine::test = 0;

	int Engine::frame = 0;
	int Engine::frameStart = 0;

	bool Engine::init(std::string name, int width, int height)
	{
		if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		{
			std::cout << "Could not initialize SDL." << std::endl;
			return false;
		}

		if (!(IMG_Init(IMG_INIT_FLAGS) && IMG_INIT_FLAGS))
		{
			std::cout << "Could not initialize SDL_image." << std::endl;
			return false;
		}

		m_Coordinator = new Coordinator();
		m_Coordinator->Init();

		m_Coordinator->RegisterComponent<Transform>();
		m_Coordinator->RegisterComponent<Renderable>();

		std::shared_ptr<RenderSystem> renderSystem = m_Coordinator->RegisterSystem<RenderSystem>();
		renderSystem->getWindow().setMode(name, width, height);
		Signature renderSignature;
		renderSignature.set(m_Coordinator->GetComponentType<Transform>());
		renderSignature.set(m_Coordinator->GetComponentType<Renderable>());
		m_Coordinator->SetSystemSignature<RenderSystem>(renderSignature);

		Funny::ResourceManager::loadPrimitives();
		Funny::ResourceManager::loadSDLTexture("assets/quote.png", "Quote");
		Funny::ResourceManager::loadSDLTexture("assets/PrtCave.png", "CaveTileset");

		return true;
	}

	bool Engine::gameLoop()
	{
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				return false;
			}
		}

		// Temp timestep stuff until a proper physics system is implemented
		if (SDL_GetTicks64() - frameStart >= 16)
		{
			frame++;
			frameStart = SDL_GetTicks64();

			m_Coordinator->GetSystem<RenderSystem>()->RenderClear();

			m_Coordinator->UpdateSystems();

			m_Coordinator->GetSystem<RenderSystem>()->RenderPresent();
		}
		
		return true;
	}

	bool Engine::close()
	{
		ResourceManager::unloadAllTextures();

		delete(test);
		delete(m_Coordinator);

		IMG_Quit();
		SDL_Quit();

		return true;
	}
}