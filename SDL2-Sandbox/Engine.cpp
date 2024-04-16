#include "Engine.h"
#include "RenderSystem.h"
#include "ResourceManager.h"

namespace Funny
{
	Engine* Engine::m_Instance = 0;
	Coordinator* Engine::m_Coordinator = 0;
	Tilemap* Engine::test = 0;

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

		Funny::ResourceManager::loadSDLTexture("assets/quote.png", "Quote");
		Funny::ResourceManager::loadSDLTexture("assets/PrtCave.png", "CaveTileset");

		Entity testEnt = m_Coordinator->CreateEntity();
		Transform transform
		{
			transform.position = Vector2(0, 0),
			transform.scale = Vector2(50, 50)
		};

		Renderable renderable
		{
			renderable.texture = ResourceManager::getSDLTexture("Quote"),
			renderable.sourceRect.x = 0,
			renderable.sourceRect.y = 0,
			renderable.sourceRect.w = 16,
			renderable.sourceRect.h = 16
		};
		m_Coordinator->AddComponent<Transform>(testEnt, transform);
		m_Coordinator->AddComponent<Renderable>(testEnt, renderable);

		Entity testEnt2 = m_Coordinator->CreateEntity();
		Transform transform2
		{
			transform2.position = Vector2(30, 0),
			transform2.scale = Vector2(50, 50)
		};

		Renderable renderable2
		{
			renderable2.texture = ResourceManager::getSDLTexture("Quote"),
			renderable2.sourceRect.x = 0,
			renderable2.sourceRect.y = 0,
			renderable2.sourceRect.w = 16,
			renderable2.sourceRect.h = 16
		};
		m_Coordinator->AddComponent<Transform>(testEnt2, transform2);
		m_Coordinator->AddComponent<Renderable>(testEnt2, renderable2);

		//test = new Tilemap();
		//test->loadMap();
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