#include "ECSDemoSystem.h"
#include "Engine.h"
#include "ResourceManager.h"

#include <random>

/*
* Throwing together this silly little demo has
* posed a lot of questions about the system as
* a whole as far as handling hitting the entity
* or component limit. I don't want to crash the
* program if the entity limit is hit, so there
* needs to be a way to properly handle that
* happening in an elegant manner.
* 
* My first idea was to return an index of -1 and
* have all functions relating processing an Entity
* that is less than 0 and greater than the limit
* to just not do anything.
* 
* This would also have to be accompanied by any
* of our component arrays not adding a component
* if the component is being added to an invalid
* entity. Currently our component arrays only
* assert that the given Entity doesn't already
* exist within the component array, but doesn't
* do anything to ensure that the given Entity
* ID actually corresponds to an existing entity.
* 
* I'm not sure how much overhead doing that sort
* of check would add though. It'd probably be better
* to do it from the coordinator since it has access
* to both the EntityManager and the ComponentManager.
* We can probably at least add a check that ensures
* that a given entity ID is in bounds (greater than
* -1 and less than the limit) from there. That way,
* we at least have a way of gracefully stopping
* the program from creating an entity that can
* NEVER exist.
* 
* That said, we don't have much of a way of stopping
* the engine from attaching components to an entity
* that doesn't actually exist yet. Worst case scenario
* is that we attach a component to an Entity that
* doesn't exist yet (ie. it still exists in the 
* EntityManager's AvailableEntityIDs queue), so the
* Entity will exist as its registered components but
* can still be grabbed from the EntityManager as a
* fresh Entity. When that happens, we'll just end up
* grabbing an Entity with components already assigned
* to it. This won't be an issue as far as signatures
* are concerned since adding components updates the
* Entity's signature based on its existing one anyways.
* Soooo, in short, it won't actually break anything,
* but it'd just be a little funky.
* 
* Fortunately, we can hide all this behind scripting
* abstracting out any actual interaction with the
* ECS directly, so no need to worry about the user
* somehow managing to attach components to non-existent
* entities and creating a new game object that already
* has stuff attached to it.
* 
* If we wanted to be extra safe though, we could have
* our Coordinator attempt to delete any components
* attached to an Entity when we first create it, just
* incase anything has been leftover somehow despite
* deleting the Entity clearing its signature and its
* attached components.
*/

/*
* Another issue thats popped up is deleting entities
* mid-update. Because deleting an entity messes with
* iterating through the ManagedEntities (since a for
* each loop can't adjust to its containers' size
* changing due to it assuming it will remain constant
* throughout, resulting in a crash from stepping out
* of the set's bounds due to the set now being smaller
* than the loop is expecting), we're going to have to
* find a workaround that deletes Entities in a way that
* doesn't interfere with update loops. I think the simplest
* and most effective way to do this would be to delete
* entities after all simulation is completed, adding
* entities that need to be delete to a "MarkedForDeletion"
* set or something. Instead of deleting the Entity right
* away, DestroyEntity would instead add the entity to
* the delete set when called. Once all Systems have
* been updated, then we go through the set and actually
* delete those entities.
* 
* I think this would also help a ton with networking
* since we could queue entities for deletion, but
* only delete them if they're marked for deletion on
* the server too or something. IDK maybe it could be
* good for events too? Or general client-server validation?
*/
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
				//newRenderable.sourceRect.w = 256;
				//newRenderable.sourceRect.h = 256;

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
	}
}