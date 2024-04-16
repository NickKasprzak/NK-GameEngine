#pragma once
#include <cassert>
#include <array>
#include <unordered_map>

#include "Types.h"

namespace Funny
{
	/*
	* An interface class that only exists
	* so we can create a list of our
	* templated component arrays.
	*
	* We can't create a list of templated
	* types, but I don't know entirely why.
	* Is it because a templated class can
	* only have things generated from it
	* if it has its type defined (since it
	* doesn't actually have a body generated
	* for it until that happens)?
	*/
	class IComponentArray
	{
	public:
		// We use a virtual deconstructor here so all ComponentArrays also delete their interfaces
		virtual ~IComponentArray() = default;
		virtual void EntityDestroyed(Entity target) = 0;
	};

	/*
	* A class to keep track of all of
	* our components of a given type,
	* ensuring that they're all compact
	* and no stale data exists between
	* any active ones.
	*/
	template <typename T>
	class ComponentArray : public IComponentArray
	{
	public:

		/*
		* Adds a component to our component array
		* alongside the Entity its being attached to.
		* This also updates the index-Entity mapping.
		*/
		void InsertComponent(Entity entity, T component)
		{
			assert(m_EntityToID.find(entity) == m_EntityToID.end() && "This Entity already has this component!");

			m_Components[m_ActiveCount] = component;
			m_IDToEntity[m_ActiveCount] = entity;
			m_EntityToID[entity] = m_ActiveCount;
			m_ActiveCount++;
		}

		/*
		* Removes the component data of the templated
		* type from the given Entity. This involves
		* resorting our component array to fill gaps
		* left by the removed component data so it
		* remains compact. This also involves updating
		* the index-Entity mappings accordingly.
		*/
		void RemoveComponent(Entity entity)
		{
			assert(m_EntityToID.find(entity) != m_EntityToID.end() && "This Entity doesn't have this component type!");

			/*
			* Keep track of the data we're working with
			*/
			int compIndexTarget = m_EntityToID[entity];
			int compIndexLast = m_ActiveCount - 1;
			Entity entityLastComp = m_IDToEntity[compIndexLast];

			/*
			* Remove component data from the target Entity by
			* overwriting it with the data from the last
			* component in the array. This also keeps our
			* component data compact together with no gaps
			* or stale data between them.
			*/
			m_Components[compIndexTarget] = m_Components[compIndexLast];

			/*
			* Since the given Entity no longer has the component
			* of this array's type, we remove its key from the
			* EntityToIndex map.
			* 
			* We also remove the index to Entity pairing of the
			* old last index since it now just contains stale data.
			*/
			m_EntityToID.erase(entity);
			m_IDToEntity.erase(compIndexLast);

			/*
			* We also need to update the Entity-index pairing of
			* the overwritten "deleted" index and the moved Entity
			* so they both refer to their new positions.
			*/
			m_EntityToID[entityLastComp] = compIndexTarget;
			m_IDToEntity[compIndexTarget] = entityLastComp;

			/*
			* Lastly we decrement active count to account for having
			* one less component and to avoid incrementing into
			* what is now stale data.
			*/
			m_ActiveCount--;
		}

		/*
		* Gets the component data of the templated
		* type corresponding to the given Entity.
		* We do this by indexing to it in the array
		* through the value returned from the Entity
		* to index map.
		*/
		T& GetComponent(Entity entity)
		{
			assert(m_EntityToID.find(entity) != m_EntityToID.end() && "This Entity doesn't have this component type!");

			return m_Components[m_EntityToID[entity]];
		}

		/*
		* Since this function is called when a given
		* Entity is deleted, we only have to have this
		* function remove the component that was once
		* attached to this Entity.
		*/
		void EntityDestroyed(Entity entity) override
		{
			if (m_EntityToID.find(entity) != m_EntityToID.end())
			{
				RemoveComponent(entity);
			}
		}

	private:
		std::array<T, MAX_ENTITIES> m_Components{};		// Where we keep all of our components
		std::unordered_map<int, Entity> m_IDToEntity{}; // Lets us get the Entity a component is attached to based on its index
		std::unordered_map<Entity, int> m_EntityToID{};	// Lets us get the index of a component attached to a given Entity through the Entity
		int m_ActiveCount = 0;							// Keeps track of all active components so we know when to stop looping through the compact array
	};
}