#pragma once
#include <unordered_map>
#include <memory>

#include "System.hpp"

namespace Funny
{
	/*
	* Our system manager keeps track of
	* all of our systems alongside holding
	* onto the signatures of each system.
	* 
	* We manage each system based on their
	* respective signatures and the signature
	* of a given entity, only having said
	* entity be referenced by a system if
	* the system's signature is present in
	* the entity's signature. We also remove
	* entities from a system if it no longer
	* contains the signature of a system
	* tracking it.
	*/
	class SystemManager
	{
	public:
		/*
		* Create a new system of the given
		* templated type as a shared pointer,
		* using its type name as an index.
		*/
		template <typename T>
		std::shared_ptr<T> RegisterSystem()
		{
			const char* typeName = typeid(T).name();

			assert(m_Systems.find(typeName) == m_Systems.end() && "That system has already been registered!");

			std::shared_ptr<T> sys = std::make_shared<T>();
			m_Systems[typeName] = sys;
			return sys;
		}

		/*
		* Return a reference to a system
		* of the templated type.
		*/
		template <typename T>
		std::shared_ptr<T> GetSystem()
		{
			const char* typeName = typeid(T).name();

			assert(m_Systems.find(typeName) != m_Systems.end() && "That system hasn't been registered yet!");

			return std::static_pointer_cast<T>(m_Systems[typeName]);
		}

		/*
		* Set the signature of the given
		* templated type.
		*/
		template <typename T>
		void SetSignature(Signature signature)
		{
			const char* typeName = typeid(T).name();

			assert(m_Systems.find(typeName) != m_Systems.end() && "That system hasn't been registed!");

			m_Signatures[typeName] = signature;
		}

		/*
		* Remove the given Entity from any
		* systems its been attached to.
		* 
		* Since a set can only have unique
		* variations of a given type, we
		* don't need to check if this Entity
		* exists in the System's entity set
		* and delete in O(1) time regardless
		* of if it actually exists there.
		*/
		void EntityDestroyed(Entity entity)
		{
			for (auto const& sysEntry : m_Systems)
			{
				auto const& sys = sysEntry.second;

				sys->m_ManagedEntities.erase(entity);
			}
		}

		/*
		* Adds or removes an entity from all
		* sets depending on what its signature
		* has been changed to and if it still
		* contains the required components to
		* be processed by the System properly.
		*/
		void EntitySignatureChanged(Entity entity, Signature signature)
		{
			for (auto const& sysEntry : m_Systems)
			{
				const char* typeName = sysEntry.first;
				std::shared_ptr<System> sys = sysEntry.second;
				Signature sysSignature = m_Signatures[typeName];

				// If the new Entity signature contains the 
				// system's signature, add it to the system
				if ((signature & sysSignature) == sysSignature)
				{
					sys->m_ManagedEntities.insert(entity);
				}

				// Otherwise, remove it
				else
				{
					sys->m_ManagedEntities.erase(entity);
				}

				// Once again, sets don't require that additional
				// check to see if an Entity exists within it.
			}
		}

		// Loops through each of our systems and runs their update loop
		void UpdateSystems()
		{
			for (auto const& sys : m_Systems)
			{
				sys.second->Update();
			}
		}

	private:
		/* 
		* Unordered maps to track the signatures
		* and systems associated with the name
		* of a given templated system type,
		* letting us grab a system's object
		* instance and its given signature based
		* on just a templated type.
		*/
		std::unordered_map<const char*, Signature> m_Signatures;
		std::unordered_map<const char*, std::shared_ptr<System>> m_Systems;
	};
}