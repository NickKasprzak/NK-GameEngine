#include <memory>

#include "ComponentArray.hpp"

namespace Funny
{
	/*
	* A class that serves the purpose
	* of managing all of our component
	* arrays, letting us add, remove,
	* delete components from each array
	* and register new components to
	* track in their own arrays.
	* 
	* I think its worth noting that the
	* fact that our component arrays and
	* systems are being stored as shared
	* pointers rather than as their blocks
	* of contiguously allocated memory
	* like our components isn't a big
	* deal. These types aren't going to
	* exist in high quantities and aren't
	* (ideally) going to be allocated at
	* runtime, so they won't hurt performance
	* and won't cause memory fragmentation
	* contrasting our components if they were
	* also handled this way.
	*/
	class ComponentManager
	{
	public:
		/*
		* Adds a new component array
		* to the array map of the
		* templated type, using the
		* type's name as the access
		* key. Also assigns the given
		* templated type a Component
		* Type ID for quick use in
		* Entity signatures.
		*/
		template<typename T>
		void RegisterComponent()
		{
			const char* typeName = typeid(T).name();

			assert(m_ComponentArrays.find(typeName) == m_ComponentArrays.end() && "That component has already been registered!");

			m_ComponentTypes[typeName] = m_NextTypeID;
			m_NextTypeID++;

			m_ComponentArrays[typeName] = std::make_shared<ComponentArray<T>>();
		}

		/*
		* Gets the ComponentType ID
		* associated with the provided
		* component.
		*/
		template<typename T>
		ComponentType GetComponentType()
		{
			const char* typeName = typeid(T).name();

			assert(m_ComponentTypes.find(typeName) != m_ComponentTypes.end() && "That component hasn't been registered yet!");

			return m_ComponentTypes[typeName];
		}

		/*
		* Inserts the given component and
		* the entity its being attached to
		* into the templated types corresponding
		* component array.
		*/
		template<typename T>
		void InsertComponent(Entity entity, T component)
		{
			GetComponentArray<T>()->InsertComponent(entity, component);
		}

		/*
		* Removes the component of the given
		* templated type from the given entity
		* through its corresponding component
		* array.
		*/
		template<typename T>
		void RemoveComponent(Entity entity)
		{
			GetComponentArray<T>()->RemoveComponent(entity);
		}

		/*
		* Retrieves a reference to the component
		* attached to the given entity of the
		* given templated type from its corresponding
		* component array.
		*/
		template<typename T>
		T& GetComponent(Entity entity)
		{
			return GetComponentArray<T>()->GetComponent(entity);
		}

		/*
		* Loop through each of the component arrays
		* and let them know that the given entity
		* is being deleted, allowing for them to
		* remove their component type from it if
		* it has one.
		*/
		void EntityDestroyed(Entity entity)
		{
			// For each component array in the component array map...
			for (auto const& strArrPair : m_ComponentArrays)
			{
				// Automatically casts the second element of the given
				// pair to a the component array interface, letting us
				// call EntityDestroyed.
				auto const& compArray = strArrPair.second;

				compArray->EntityDestroyed(entity);
			}
		}

	private:
		std::unordered_map<const char*, ComponentType> m_ComponentTypes;					 // Keeps track of what Type ID corresponds to each component
		std::unordered_map<const char*, std::shared_ptr<IComponentArray>> m_ComponentArrays; // Keeps track the component array associated with each component
																							 // We manage the arrays as shared pointers so we can pass them around safely

		ComponentType m_NextTypeID = 0;														 // Keeps track of the next ID to assign to a newly registered component

		/*
		* A convenience function that handles
		* grabbing the component array of the
		* given templated component type from the
		* array map without having to manually
		* get the name from the templated type
		* each time.
		*/
		template<typename T>
		std::shared_ptr<ComponentArray<T>> GetComponentArray()
		{
			const char* typeName = typeid(T).name();

			assert(m_ComponentArrays.find(typeName) != m_ComponentArrays.end() && "That component hasn't been registered yet!");

			// We need to cast to the specific array type so we don't return the pointer as the interface class.
			return std::static_pointer_cast<ComponentArray<T>>(m_ComponentArrays[typeName]);
		}
	};
}