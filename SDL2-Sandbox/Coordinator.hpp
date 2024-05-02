#pragma once

#include "EntityManager.hpp"
#include "ComponentManager.hpp"
#include "SystemManager.hpp"

// MOVE THIS SHIT ELSEWHERE HOLY FUCK

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
* I'm still unsure as to how to ensure that entities have the
* same network ID across all instances of the game. Maybe have
* the network system manage all networked instances from itself.
* The network system could have a "CreateNetworkedEntity" func
* that creates an entity through the coordinator like normal, but
* also assigns that Entity to a network ID, either being created
* server side and picks an available ID that the Entity can use,
* or client side and assigns an ID to the newly created Entity
* based on the ID contained in the packet when recieved from the
* server. The client shouldn't be creating a networked entity,
* only asking the server to create one for it. We want the
* server to authoritative over what happens in-game. This applies
* to deletion as well. The client should only delete networked
* entities if the server says its ok.
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
* 
* Transforms in themselves shouldn't ALWAYS be communicated.
* I'm thinking a better approach would be to instead communicate
* entire entities over the wire, with their Networked Entity ID
* and the components they have serialized into our packets. The
* entities that do get communicated over the wire depends entirely
* on the game state, so if something changes with an Entity on
* the server that must be replicated across all connected clients,
* then we send a packet with that Entity's updated component data.
* For example, entities with active physics states would need to
* have their physics data sent to each client. This would include
* their new transform from physics simulation.
* 
* Queue entities that need to be updated across the network in
* networking system from other systems (ie. PhysicsSystem tells
* NetworkSystem about the entities it updated that frame and what
* components it changed so the NetworkSystem can construct packets
* from specifically just that data.) This way we won't have to have
* our NetworkSystem look through every single Entity, check all of
* their components based on their signatures, and then create a
* packet from all their data, without even knowing if it was changed
* at all from the previous state. This way we can have a reliable
* way of telling our network system what data it should spent its
* time replicating and what data can be left alone (ie. something
* with a transform that hasn't actually changed since last state).
* 
* Handling things this way also lets us create and store world states
* based on what was queued to be updated across the network based on
* entity and component data, which can be useful for things like
* resending lost data, state interpolation, validating client requests,
* making sure we're only updating relevant data across the network,
* and so on.
* 
* Going more into detail about queueing things to be replicated
* across the network by system, this lets us decouple networking
* from our components. Not everything with a Transform needs to
* be updated across the network every update as not everything with
* a Transform is updated within the game frequently.
* 
* My only worry about this system is handling updates to components
* that aren't directly linked to a major system like Physics. What
* if the player changes an object's transform directly? How do we
* know to communicate that it was changed and needs to be replicated
* or validated by the server? The idea of putting network code into
* every single component and ensuring they can only be updated through
* setter functions makes me sick since we want to keep things as
* decoupled as possible. Checking and updating every single component
* also just isn't feasible since that would put too much strain on
* the network and the server.
*
* But I also feel like this crosses the line between engine logic
* and game logic. The player shouldn't have access to directly changing
* crucial values like transform data directly outside of something
* like a scripting API, and a scripting API can handle updating things
* within the engine and telling the network system that something has
* been changed via script and needs to be replicated/validated. Not
* everything needs to be exactly replicated across the network and
* if the player does attempt to inject modifications to a component's
* data directly (ie. update Transform position data) through unintended
* means, they'll end up desyncing with the server on their own machine
* through their own doing and will only disrupt/change the play experience
* for themselves, keeping other players and the server safe from their
* stupid antics. Keeping the scripting API safe from hacking the game
* is its own can of worms though, that would be thinking toooo far ahead.
* 
* That all being said, we could notify the network system/manager that
* an object that needs to be replicated has to be updated, so instead
* of accessing the network system directly from our other systems'
* update functions, we can just shoot out an event saying that the
* given entity and some of its components were updated and anything
* that cares (like our networking system) will hear this and respond.
* That way, our physics and networking systems remain entirely decoupled,
* alongside various other systems that might need to know about state
* and entity updates for their respective system processes.
* 
* But speaking of events, would it be better to handle event execution
* and script simulation entirely on the server? While it might be a little
* rough having things like weapons have their simulation tied to the server,
* I do think it'd be neccessary to prevent things like desyncs between
* the client and server from happening in the event an event is fired from
* the client's perspective but isn't from the server's. We won't have to
* worry about completely reversing the results of firing the event on the
* client if the server deems the execution of that event impossible based
* on its perspective. Given a scenario where high latency would be an issue
* to begin with, I would argue that having a delayed event execution would
* be a better experience for the player than having an event trigger and
* immediately have that trigger be reversed a second later, forcing them
* to try to trigger the event again by repeating whatever action they did.
* Its like "Click and wait a second to fire" versus "Click multiple times
* until you fire and it isn't immediately reverted by the server". Both
* aren't ideal, but waiting is the lesser of the two evils here. It'd also
* be impossible to predict how an event or script-based function would
* play out considering events can be anything and scripts can LITERALLY
* be anything provided the API offers the tools to make it happen.
* 
* For now, focus on getting a basic client-server model established.
* Have a server class that manages players, keeping track of their
* port and address (if thats even needed for TCP), what entity theyre
* controlling, and maybe username? Use a dumb client model where only
* user input gets communicated from the client, which the server then
* processes, validates, and sends back to the client as their new
* position. Have an input system of some kind, doesn't have to be
* super robust or complicated. Have a way to serialize input and
* position data. Have a way to add and remove players, adding player
* to the player list and sending them the current world state on
* connect, and removing them and their entity from the game on
* disconnect/timeout. Have a way to communicate Entity creation and
* deletion to each client from the server.
*/

/*
* thought dump from the network manager
* 
* SERVER ONLY SYSTEM (probably)
*
* Reason being that the client and server need to store and process the
* whole game loop differently.
*
* === SERVER ===
*
* Server needs to create, update, and destroy entities across the network
* based on its perception of the whole game state since everything is being
* simulated by it.
*
* Server needs to initialize a socket to listen for connections, keep track
* of said connections, and what entity those connections are represented by.
* Also needs to recieve and process input packets from those connections.
*
* Needs to keep track of the entities created, updated, and deleted during
* the gameplay loop that should be networked. It keeps track of the entities
* that need to be networked based on its signature, representing the component
* data it needs to keep an eye on to ensure that only they get replicated.
* Things like Transforms and BoxColliders (in our case for this basic game).
*
* It only needs to worry itself with things like Renderables during creation
* so it can let the client know what kind of sprite an object should have.
* Since Renderables are texture data managed by pointers, we need to tell
* the client the name of the texture being loaded. This also means that the
* ResourceManager needs to be opened on the server.
*
* During the update loop, it loops through all of the entities its managing.
* If an entity its managing doesn't have a NetworkID attached to it, it assigns
* one to it and sends out a packet to clients informing them to create an Entity
* corresponding to that network ID as well. If an entity attached to a network ID
* is no longer being managed, we can assume that the entity has been deleted.
* Except not really because an entity can become unmanaged by any system if the
* components that need to be tracked are removed. Working around not explicitly
* knowing if an entity has been deleted or is just unmanaged by the network is
* another problem an event system would fix since we can have events telling us
* whether a component was removed or an entity was deleted rather than having to
* make assumptions about it. ITS PROBABLY FINE FOR NOW????? FUCK.
*
* Have this exist seperately from the ECS systems. Store a signature of the components
* we absolutely need to replicate across the server. Have events fire for whenever
* the coordinator adds a component to an entity or deletes an entity. If the entity
* created matches the s
*
* Entity archetypes? I don't know enough to about engine programming to do networking
* for it yet lol. This is too much. I don't know enough about various engine programming
* concepts or networking to know exactly how to properly generate what aspects of a world
* state have been updated, what the server has updated and what specifically it needs to
* care about. BUT THE STUFF WE CARE ABOUT UPDATING IS THE STUFF UPDATED BY SPECIFIC SYSTEMS.
* WE CARE ABOUT PHYSICS UPDATES, WE CARE ABOUT COMPONENT CREATION AND STUFF. Anything can
* have a transform and collision box, but theres no point in communicating data about it
* over the network if the systems its used by (physics) have updated it.
*
* So have events for creating entities, assigning ALL entities corresponding NetworkIDs,
* adding components to entities, removing components from entities, updating components
* attached to entities, and deleting entities.
*
* All of this needs to be replicated across the network. It doesn't matter. What matters
* is when it happens. If the server EVER creates an object of any type, it probably needs
* to be replicated across the network.
*
* Whats throwing me off is local entities. Stuff that varies from client to client. Each
* player probably doesn't need to store info relating to the PlayerControllers for other
* players, or things like their camera. I guess the only difference would be that the
* server holds onto a NetworkID for the local entity, but the data specific to that entity
* and its components is stored locally on each client? Fuck. I'm overthinking this. FUUCCKCKK.
*
* === EVENT BENEFITS ===
*
* Itd be nice to have a queue of every updated entity or something so I only
* have to care about whats been updated during a given frame. I could do that
* through events so the server could be indirectly informed about entities
* being updated or deleted based on their IDs so we dont have to trudge through
* the entire managed list to see if an entity with a networked ID has been updated
* or deleted and how. This would also let us isolate certain types of updates
* by only registering the server to listen for specific updates it needs to be
* aware of (physics updates and deletion updates).
*
* The above would also simplify the Update loop since a list of entities that
* have been modified in some capacity would've been formed by the server's response
* to certain events.
*
* Having an event system in general would just be good. Push for it. Actually,
* don't. You're overcomplicating things.
*
* === CLIENT ===
*
*
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

		Signature GetEntitySignature(Entity entity)
		{
			return m_EntityManager->GetEntitySignature(entity);
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