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