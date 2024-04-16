#pragma once

#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "SystemManager.hpp"

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