#pragma once
#include <set>

#include "Types.h"

namespace Funny
{
	/*
	* The base system class just serves
	* as an interface for any larger and
	* more complex system functionality,
	* only containing a set of entities
	* that contain the components that
	* are held in the system's signature.
	* 
	* The signature of a given system is
	* just held in the SystemManager so
	* we don't have to jump into every
	* single System class and check its
	* signature, instead keeping all
	* System signatures in one space in
	* memory, which is cache friendly.
	* 
	* Since the System signatures are
	* also held in the SystemManagers,
	* we also add Entities to the
	* systems from there. Managing
	* a specific set of entities in
	* each system also makes the update
	* loop more efficient too, since
	* each system would ideally have
	* all the Entities it needs to work
	* with at all times, so it won't
	* need to check all entities to
	* see if it has the components it
	* needs every frame.
	* 
	* Last note, we can get the component
	* data we need to work with and update
	* through our coordinator and have
	* it call into the ComponentManager
	* to get the component associated
	* with each of the Entities we need
	* to work with.
	* 
	* Another note, we have a pure virtual
	* function to handle updating each
	* of our systems so each of them can
	* update all of their managed entities.
	* This is called for all managed systems
	* from the SystemManager and Coordinator.
	*/
	class System
	{
	public:
		virtual void Update() = 0;
		std::set<Entity> m_ManagedEntities{};
	};
}