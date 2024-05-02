#pragma once
#include "SimpleSock/socklib.h"
#include "Engine.h"

namespace Funny
{
	// Types of entity updates
	enum NetUpdateType
	{
		CREATED,
		UPDATED,
		DESTROYED,
		TYPES
	};

	// Manages player/client data
	struct ConnectionData
	{
		Socket m_Socket;
		int m_EntityNetworkID;
		int score;
	};

	typedef std::uint16_t NetworkID;
	const char* const host = "127.0.0.1";
	const int port = 2000;
	const int MAX_CONNECTIONS = 2;
	const NetworkID MAX_NETWORKED_ENTITIES = 50;

	/*
	* === SERVER ===
	* 
	* Keeps track of all entities with NetworkIDs. When an Entity is created
	* or deleted from the Coordinator, call CreateNetworkedEntity or DeleteNetworkedEntity
	* to ensure these changes are reflected through network IDs.
	* 
	* When an entity has its components updated at all (ONLY WORRY ABOUT PHYSICS FFS), call
	* UpdateNetworkedEntity to indicate that the entity has changes that need to be replicated.
	* 
	* Manage opening a socket for connections and make sure its non-blocking. Listen for connections
	* then add those connections to a list, assigning each a player Entity to control. Process
	* any input recieved from the clients and apply the result to the Entity of their corresponding
	* NetworkID. Trash any invalid data that doesn't get serialized to input.
	* 
	* Please excuse a lot of the game logic being REALLY bad. And some stuff in general being bad.
	* A lot of this is bad. I wanted to keep a lot of the game logic for the final outside of the
	* existing engine to keep things simple and easy to implement, so everything else suffers as a
	* result.
	*/
	class ServerSystem
	{
	public:
		/*
		* Initialize the server by opening and configuring a
		* socket to listen for and accept connections.
		* Using TCP for simplicity.
		*/
		ServerSystem()
		{
			m_Socket.Create(Socket::Family::INET, Socket::Type::STREAM);
			m_Address = Address(host, port);
			m_Socket.Bind(m_Address);
			
			for (NetworkID i = 0; i < MAX_NETWORKED_ENTITIES; i++)
			{
				m_AvailableNetworkIDs.push(i);
			}
		}

		~ServerSystem()
		{

		}

		static ServerSystem* createInstance()
		{
			if (m_Instance == nullptr)
			{
				m_Instance = new ServerSystem();
				return m_Instance;
			}

			return nullptr;
		}
		static ServerSystem* getInstance() { return m_Instance; }

		void ListenForConnections();
		void InitializeClients();

		void update();

		bool RecieveFromClients();
		void SendToClients();

		void SpawnCoins();
		void CheckForOverlaps();

		int CreateNetworkedEntity(Entity entity);
		void DeleteNetworkedEntity(Entity entity);
		void UpdateNetworkedEntity(Entity entity);

	private:
		static ServerSystem* m_Instance;

		// Keep track of existing entities and their corresponding network IDs
		std::unordered_map<Entity, NetworkID> m_EntityToNetworkID;
		std::unordered_map<NetworkID, Entity> m_NetworkIDToEntity;

		// Keep track of available NetworkIDs
		std::queue<NetworkID> m_AvailableNetworkIDs;

		// Keep track of all entities that need to be updated during
		// the next update and the type of update.
		std::queue<std::pair<NetworkID, NetUpdateType>> m_UpdatedEntities;

		Socket m_Socket;
		Address m_Address;

		ConnectionData m_Connections[MAX_CONNECTIONS];
		int m_ConnCount = 0;

		int m_Coins = 0;
		int m_MaxCoins = 10;
		int m_CoinTimer = 0;
		int m_CoinSpawnTime = 120;
		int m_CoinsToWin = 5;

		ColorRGBA m_CoinColor =
		{
			m_CoinColor.r = 245,
			m_CoinColor.g = 229,
			m_CoinColor.b = 39,
			m_CoinColor.a = 255
		};
	};
}