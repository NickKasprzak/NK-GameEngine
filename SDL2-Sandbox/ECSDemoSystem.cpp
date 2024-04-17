#include "ECSDemoSystem.h"
#include "Engine.h"
#include "ResourceManager.h"

#include <random>

namespace Funny
{
	ECSDemoSystem::ECSDemoSystem()
	{
		m_MaxGeneratedEntities = (m_MaxGeneratedEntities > MAX_ENTITIES) ? MAX_ENTITIES : m_MaxGeneratedEntities;
	}

	ECSDemoSystem::~ECSDemoSystem()
	{

	}

	void ECSDemoSystem::Update()
	{
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> distribPos(m_XSpawnPosMin, m_XSpawnPosMax);
		std::uniform_int_distribution<> distribCol(0, 255);
		std::uniform_int_distribution<> distribSpr(0, 2);

		if (m_GeneratedEntities < m_MaxGeneratedEntities)
		{
			Entity newEntity = Engine::getCoordinator()->CreateEntity();

			Transform newTransform;
			{
				newTransform.position.x = distribPos(gen);
				newTransform.position.y = m_YSpawnPos;
				newTransform.scale.x = 50;
				newTransform.scale.y = 50;
			};
			Engine::getCoordinator()->AddComponent<Transform>(newEntity, newTransform);


			Renderable newRenderable;
			{
				
				switch (distribSpr(gen))
				{
					case 0:
						newRenderable.texture = ResourceManager::getSDLTexture("Square");
						break;

					case 1:
						newRenderable.texture = ResourceManager::getSDLTexture("Circle");
						break;

					case 2:
						newRenderable.texture = ResourceManager::getSDLTexture("Triangle");
						break;

					default:
						break;
				}

				newRenderable.sourceRect.x = 0;
				newRenderable.sourceRect.y = 0;
				SDL_QueryTexture(newRenderable.texture, NULL, NULL, &newRenderable.sourceRect.w, &newRenderable.sourceRect.h); // Move to helper func?

				newRenderable.color.r = distribCol(gen);
				newRenderable.color.g = distribCol(gen);
				newRenderable.color.b = distribCol(gen);
				//newRenderable.color.a = distribCol(gen);
			}
			Engine::getCoordinator()->AddComponent<Renderable>(newEntity, newRenderable);

			m_GeneratedEntities++;
		}

		std::vector<int> entToDest;
		for (auto const& entity : m_ManagedEntities)
		{
			Transform& entTransform = Engine::getCoordinator()->GetComponent<Transform>(entity);
			entTransform.position.y = entTransform.position.y + 1;
			if (entTransform.position.y == m_YDespawnPos)
			{
				entToDest.push_back(entity);
				//Engine::getCoordinator()->DestroyEntity(entity);
			}
		}

		for (int i = 0; i < entToDest.size(); i++)
		{
			Engine::getCoordinator()->DestroyEntity(entToDest[i]);
			m_GeneratedEntities--;
		}

		std::cout << Engine::getFPS() << std::endl;
	}
}