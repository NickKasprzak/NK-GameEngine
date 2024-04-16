#pragma once
#include <cassert>
#include <array>
#include <queue>

#include "Types.h"

namespace Funny
{
	/*
	* A class to keep track of all of our
	* live entities and their respective
	* component signatures.
	*/
	class EntityManager
	{
	public:
		/*
		* Initializes our Entity ID queue to contain
		* all possible IDs from 0 to MAX_ENTITIES.
		*/
		EntityManager()
		{
			for (Entity i = 0; i < MAX_ENTITIES; i++)
			{
				m_AvailableEntityIDs.push(i);
			}
		}

		/*
		* "Creates" an Entity by pulling and returning
		* a fresh Entity ID from the ID queue. This also
		* increments our live entity count.
		*/
		Entity CreateEntity()
		{
			assert(m_LiveEntities < MAX_ENTITIES && "You've hit the Entity limit!");

			Entity newEntity = m_AvailableEntityIDs.front();
			m_AvailableEntityIDs.pop();
			m_LiveEntities++;

			return newEntity;
		}

		/*
		* "Destroys" an Entity by returning the given
		* Entity ID to the ID queue at the end and
		* clearing the signature associated with the
		* that ID so its fully fresh and doesn't
		* contain stale data. Also decrements the
		* live Entity count to accommodate.
		*/
		void DestroyEntity(Entity target)
		{
			assert(target < MAX_ENTITIES && "That ID is out of bounds!");

			m_Signatures[target].reset();
			m_AvailableEntityIDs.push(target);
			m_LiveEntities--;
		}

		/*
		* Returns the signature associated with the
		* given Entity by indexing into the signature
		* array and getting its associated signature
		*/
		Signature GetEntitySignature(Entity target)
		{
			assert(target < MAX_ENTITIES && "That ID is out of bounds!");

			return m_Signatures[target];
		}

		/*
		* Updates the signature associated with the
		* given Entity by indexing into the signature
		* array and reassigning the signature located
		* at the Entity's ID to the given signature.
		*/
		void SetEntitySignature(Entity target, Signature newSignature)
		{
			assert(target < MAX_ENTITIES && "That ID is out of bounds!");

			m_Signatures[target] = newSignature;
		}

	private:
		std::queue<Entity> m_AvailableEntityIDs{};			// Holds all available Entity IDs
		std::array<Signature, MAX_ENTITIES> m_Signatures{}; // Keeps track of the signatures of all Entities, using the Entity itself to index
		uint32_t m_LiveEntities = 0;						// Keeps track of how many Entities are currently active
	};
}