#pragma once

#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "SystemManager.hpp"

/*
* Throwing together the silly ECS demo has
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

/*
* When it comes to networking the ECS-related stuff, I
* think a functional approach would be to serialize and
* deserialize our objects through their Entity ID ie.
* the ID corresponding to each entity/component grouping
* will be the exact same on the client and server. An
* entity of ID 1 that has data for a transform at (10, 10)
* on the server will have that exact data on all connected
* clients as well.
*
* The only problem with this approach is how Entities are
* created. Since the EntityManager just grabs the first
* available Entity from a queue on creation, we can't just
* say "oh Entity of ID n has these components!" without
* messing up the internal system. Instead, we could have
* global Network IDs attached to a local Entity ID, so
* Entity IDs don't matter and all changes to a local entity
* are done based on its global Network ID'd counterpart.
* For example, an Entity with Network ID 2 could have a
* transform of (10, 10) and an Entity ID of 10 on the server
* but an Entity ID of 5 on the client. The server tells
* the client to update the Entity of Network ID 2 with
* new transform data, so the client updates Entity of ID
* 5 with the new transform data. Both represent the same
* exact data visually, but can be stored at different
* positions in memory locally without issue.
* 
* This also lets us correct any issues (like premature
* deletion or component additions/removals) by replicating
* the deserialized data provided by the server at any entity
* as long as said entity corresponds to the given Network ID
* on all clients.
* 
* This also gives us a clearer picture of what we should be
* sending in our packets across the server, being the Network
* ID of the entity and the components we want to update. We
* can pick and choose what we send and don't send to the client
* as long as the game state gets replicated properly.
* 
* Long term example, but components like transforms should
* always be communicated across the network since they're
* pretty much always changing. But things like Sprites should
* only be passed over once (somehow through sending the image
* data as bytes) to be loaded by the ResourceManager locally
* so it can then be replicated by sending the image's name
* as the component's data rather than always sending the full
* sprite each time. We could do something like "Entity2 has
* sprite 'Quote'". If the client doesn't have sprite 'Quote'
* loaded, it could ask the server to send it the sprite data
* to then attach to Entity2 once the sprite is loaded,
* replicating everything else about Entity2 in the meantime,
* including providing it an empty Renderable component with
* the other needed data as a placeholder.
*/
namespace Funny
{
	/*
	* A class to manage our managers
	* and make sure all operations
	* are handled in sync across each.
	*/
	class Coordinator
	{
	public:
		/*
		* Initialize each of our systems
		*/
		void Init()
		{
			m_EntityManager = std::make_unique<EntityManager>();
			m_ComponentManager = std::make_unique<ComponentManager>();
			m_SystemManager = std::make_unique<SystemManager>();
		}

		/*
		* Handle entities
		*/
		Entity CreateEntity()
		{
			return m_EntityManager->CreateEntity();
		}

		void DestroyEntity(Entity entity)
		{
			m_EntityManager->DestroyEntity(entity);
			m_ComponentManager->EntityDestroyed(entity);
			m_SystemManager->EntityDestroyed(entity);
		}


		/*
		* Handle components
		*/
		template<typename T>
		void RegisterComponent()
		{
			m_ComponentManager->RegisterComponent<T>();
		}

		template<typename T>
		void AddComponent(Entity entity, T component)
		{
			m_ComponentManager->InsertComponent<T>(entity, component);

			Signature entSignature = m_EntityManager->GetEntitySignature(entity);
			ComponentType compType = m_ComponentManager->GetComponentType<T>();
			entSignature.set(compType, true);
			m_EntityManager->SetEntitySignature(entity, entSignature);

			m_SystemManager->EntitySignatureChanged(entity, entSignature);
		}

		template<typename T>
		void RemoveComponent(Entity entity)
		{
			m_ComponentManager->RemoveComponent<T>();

			Signature entSignature = m_EntityManager->GetEntitySignature(entity);
			ComponentType compType = m_ComponentManager->GetComponentType<T>();
			entSignature.set(compType, false);
			m_EntityManager->SetEntitySignature(entity, entSignature);

			m_SystemManager->EntitySignatureChanged(entity, entSignature);
		}

		template<typename T>
		T& GetComponent(Entity entity)
		{
			return m_ComponentManager->GetComponent<T>(entity);
		}

		template<typename T>
		ComponentType GetComponentType()
		{
			return m_ComponentManager->GetComponentType<T>();
		}

		/*
		* Handle systems
		*/
		template<typename T>
		std::shared_ptr<T> RegisterSystem()
		{
			return m_SystemManager->RegisterSystem<T>();
		}

		template<typename T>
		std::shared_ptr<T> GetSystem()
		{
			return m_SystemManager->GetSystem<T>();
		}

		template<typename T>
		void SetSystemSignature(Signature signature)
		{
			m_SystemManager->SetSignature<T>(signature);
		}

		void UpdateSystems()
		{
			m_SystemManager->UpdateSystems();
		}

	private:
		std::unique_ptr<EntityManager> m_EntityManager;
		std::unique_ptr<ComponentManager> m_ComponentManager;
		std::unique_ptr<SystemManager> m_SystemManager;
	};
}