#include "ServerSystem.h"
#include "ByteStream.h"
#include "PacketTypes.h"
#include <random>

namespace Funny
{
	ServerSystem* ServerSystem::m_Instance = 0;

	void ServerSystem::ListenForConnections()
	{
		std::cout << "Waiting for connections..." << std::endl;
		m_Socket.Listen();

		while (m_ConnCount < MAX_CONNECTIONS)
		{
			Socket& clientSock = m_Connections[m_ConnCount].m_Socket;
			m_Socket.AcceptInto(clientSock);
			clientSock.SetNonBlockingMode(true);
			m_ConnCount++;
			std::cout << "Connection recieved. Current connections are " << m_ConnCount << std::endl;
		}

		std::cout << "Max players connected. Starting game." << std::endl;

		InitializeClients();

		m_Socket.SetNonBlockingMode(true); // probably doesnt do anything.
	}

	/*
	* Create a player entity for each active
	* client to control, sending them packets
	* to create each, then telling them which
	* entity they will control through their
	* respective NetworkID.
	*/
	void ServerSystem::InitializeClients()
	{
		std::random_device rand;
		std::mt19937 gen(rand());

		for (int i = 0; i < m_ConnCount; i++)
		{
			// Create new player entity w/ a transform
			// and renderable components
			Entity newEnt = Engine::getCoordinator()->CreateEntity();

			Transform entTransform;
			std::uniform_int_distribution<> randPos(0, 400);
			entTransform.position = Vector2(randPos(gen), randPos(gen));
			entTransform.scale = Vector2(50, 50);
			Engine::getCoordinator()->AddComponent<Transform>(newEnt, entTransform);

			Renderable entRenderable;
			entRenderable.sourceRect.x = 0;
			entRenderable.sourceRect.y = 0;
			entRenderable.sourceRect.w = 256;
			entRenderable.sourceRect.h = 256;

			std::uniform_int_distribution<> randCol(0, 255);
			entRenderable.color = ColorRGBA(randCol(gen), randCol(gen), randCol(gen), 255);

			std::uniform_int_distribution<> randShape(0, 2);
			int shapeNum = randShape(gen);
			switch (shapeNum)
			{
				case 0:
					memcpy(entRenderable.name, "Square", 6);
					entRenderable.nameLen = 6;
					break;

				case 1:
					memcpy(entRenderable.name, "Circle", 6);
					entRenderable.nameLen = 6;
					break;

				case 2:
					memcpy(entRenderable.name, "Triangle", 8);
					entRenderable.nameLen = 8;
					break;
			}
			Engine::getCoordinator()->AddComponent<Renderable>(newEnt, entRenderable);

			m_Connections[i].m_EntityNetworkID = CreateNetworkedEntity(newEnt);
			m_Connections[i].score = 0;
		}
	}

	void ServerSystem::update()
	{
		RecieveFromClients();

		// This stupid loop here mightve exposed an issue with the
		// ECS where no safety measures exist to ensure an entity
		// ID is in use before deleting it, leading to the live
		// entity count underflowing to its maximum value.
		// 
		// Wow my first underflow error! Cool!
		for (int i = 0; i < m_ConnCount; i++)
		{
			ConnectionData& conn = m_Connections[i];
			if (conn.score == m_CoinsToWin)
			{
				std::cout << "Client " << i << " has won. Restarting." << std::endl;

				for (auto const& val : m_EntityToNetworkID)
				{
					Entity ent = val.first;
					DeleteNetworkedEntity(ent);
				}

				InitializeClients();
				SendToClients();
				return;
			}
		}

		CheckForOverlaps();

		SpawnCoins();

		SendToClients();
	}

	bool ServerSystem::RecieveFromClients()
	{
		// Recieve input from connections.
		for (int i = 0; i < m_ConnCount; i++)
		{
			char buff[4096];
			int bytesRecv = m_Connections[i].m_Socket.Recv(buff, 4096);

			if (bytesRecv == 0)
			{
				std::cout << "Connection hung up. Ending game." << std::endl;
				return false;
			}

			else if (bytesRecv == -1)
			{
				if (m_Connections[i].m_Socket.GetLastError() != Socket::Error::SOCKLIB_EWOULDBLOCK)
				{
					abort();
				}

				continue;
			}

			else if (bytesRecv > 0)
			{
				// Increment transform position of entity corresponding
				// to the client that sent the message and add that entity
				// to the update entities queue to be replicated across clients

				// should be as simple as just taking the entity at the netID
				// of the current ConnectionData, grabbing its transform, and
				// adding to it in whatever direction the client is moving.

				ByteStream stream;
				stream.setBuffer(buff, bytesRecv);

				NetworkID cliNetID = m_Connections[i].m_EntityNetworkID;
				Entity cliEntity = m_NetworkIDToEntity[cliNetID];
				bool cliEntUpdated = false;

				while (!stream.endOfStream())
				{
					PacketHeader hPacket;
					hPacket.DeserializePacket(stream);

					if (hPacket.packetType == PacketTypes::CLIENT_INPUT)
					{
						ClientInputPacket iPacket;
						iPacket.DeserializePacket(stream);
						Transform& entTransform = Engine::getCoordinator()->GetComponent<Transform>(cliEntity);

						switch (iPacket.inputDir)
						{
						case InputDirections::UP:
							entTransform.position.y -= 10;
							break;

						case InputDirections::DOWN:
							entTransform.position.y += 10;
							break;

						case InputDirections::LEFT:
							entTransform.position.x -= 10;
							break;

						case InputDirections::RIGHT:
							entTransform.position.x += 10;
							break;
						}

						cliEntUpdated = true;
					}
				}

				if (cliEntUpdated)
				{
					UpdateNetworkedEntity(cliEntity);
				}
			}
		}

		return true;
	}

	/*
	* Server sends packets to each client through
	* the packet manager for each entity that has
	* had some kind of update made to it since
	* last state update.
	* 
	* This works fine provided the small scope of
	* the game, but it doesn't handle creation and
	* deletion of objects rapidly in one frame well.
	* If an event for an entity goes out for its
	* creation and deletion on the same frame, then
	* the client wont know what to do since it could
	* recieve a delete event for something it hasn't
	* even recieved yet.
	* 
	* Its just something good to keep in mind for
	* the future, just need a more strict and secure
	* system to avoid issues like this. And one that
	* hasn't been cobbled together in a few days.
	*/
	void ServerSystem::SendToClients()
	{
		while (!m_UpdatedEntities.empty())
		{
			// Get local entity values
			auto const& data = m_UpdatedEntities.front();
			NetworkID netID = data.first;
			NetUpdateType updateType = data.second;
			Entity localEnt = m_NetworkIDToEntity[netID];
			Signature localSig = Engine::getCoordinator()->GetEntitySignature(localEnt);

			// Packet byte stream
			ByteStream packetStream;

			// Load packet header
			PacketHeader packHeader;
			packHeader.packetType = PacketTypes::ENTITY_UPDATE;
			packHeader.SerializePacket(packetStream);

			// Load entity header
			EntityPacketHeader entHeader;
			entHeader.networkID = netID;
			entHeader.updateType = updateType;
			entHeader.signature = localSig;
			entHeader.SerializePacket(packetStream);

			// Load packet with comp data if not destroy update
			// Terrible way to handle this but it works.
			// Would a packet factory do something like this?
			if (updateType != NetUpdateType::DESTROYED)
			{
				Signature tempSig;

				// Check for and serialize transform
				ComponentType transformType = Engine::getCoordinator()->GetComponentType<Transform>();
				tempSig.flip(transformType);
				if ((localSig & tempSig) != 0)
				{
					Transform entTrans = Engine::getCoordinator()->GetComponent<Transform>(localEnt);
					TransformPacket tPack;
					tPack.position.x = entTrans.position.x;
					tPack.position.y = entTrans.position.y;
					tPack.scale.x = entTrans.scale.x;
					tPack.scale.y = entTrans.scale.y;
					tPack.SerializePacket(packetStream);
				}
				tempSig.reset();

				// Check for renderable
				ComponentType renderableType = Engine::getCoordinator()->GetComponentType<Renderable>();
				tempSig.flip(renderableType);
				if ((localSig & tempSig) != 0)
				{
					Renderable entRend = Engine::getCoordinator()->GetComponent<Renderable>(localEnt);
					RenderablePacket rPack;
					memcpy(rPack.name, entRend.name, entRend.nameLen);
					rPack.nameSize = entRend.nameLen;
					rPack.sourceRect.x = entRend.sourceRect.x;
					rPack.sourceRect.y = entRend.sourceRect.y;
					rPack.sourceRect.w = entRend.sourceRect.w;
					rPack.sourceRect.h = entRend.sourceRect.h;
					rPack.color.r = entRend.color.r;
					rPack.color.g = entRend.color.g;
					rPack.color.b = entRend.color.b;
					rPack.color.a = entRend.color.a;
					rPack.SerializePacket(packetStream);
				}
				tempSig.reset();
			}

			// Get packet data from stream
			char packet[4096];
			size_t packetBytes;
			packetStream.getBuffer(packet, 4096, packetBytes);

			// Send entity update packet out to clients
			for (int i = 0; i < m_ConnCount; i++)
			{
				m_Connections[i].m_Socket.Send(packet, packetBytes);
			}

			// Add net ID of deleted entity back to queue if deleted
			// to make 100% sure the ID doesnt get reused before the
			// delete packet arrives
			if (updateType == NetUpdateType::DESTROYED)
			{
				m_EntityToNetworkID.erase(localEnt);
				m_NetworkIDToEntity.erase(netID);
				m_AvailableNetworkIDs.push(netID);
			}

			// Remove entity update from queue
			m_UpdatedEntities.pop();
		}
	}

	void ServerSystem::CheckForOverlaps()
	{
		for (int i = 0; i < m_ConnCount; i++)
		{
			ConnectionData& conn = m_Connections[i];
			Entity pEnt = m_NetworkIDToEntity[conn.m_EntityNetworkID];
			Transform& pTransform = Engine::getCoordinator()->GetComponent<Transform>(pEnt);
			Renderable& pRenderable = Engine::getCoordinator()->GetComponent<Renderable>(pEnt);

			Signature tempSig;
			ComponentType transformType = Engine::getCoordinator()->GetComponentType<Transform>();
			tempSig.flip(transformType);
			ComponentType renderableType = Engine::getCoordinator()->GetComponentType<Renderable>();
			tempSig.flip(renderableType);

			for (auto const& val : m_EntityToNetworkID)
			{
				Entity ent = val.first;
				if ((Engine::getCoordinator()->GetEntitySignature(ent) & tempSig) != 0)
				{
					Transform& cTransform = Engine::getCoordinator()->GetComponent<Transform>(ent);
					Renderable& cRenderable = Engine::getCoordinator()->GetComponent<Renderable>(ent);

					if (cRenderable.color == m_CoinColor &&
						cTransform.position.x < pTransform.position.x + (pTransform.scale.x/2) &&
						cTransform.position.x > pTransform.position.x - (pTransform.scale.x/2) &&
						cTransform.position.y < pTransform.position.y + (pTransform.scale.y/2) &&
						cTransform.position.y > pTransform.position.y - (pTransform.scale.y/2))
					{
						conn.score++;
						m_Coins--;
						DeleteNetworkedEntity(ent);
					}
				}
			}
		}
	}

	void ServerSystem::SpawnCoins()
	{
		if ((m_Coins != m_MaxCoins) && (m_CoinTimer == m_CoinSpawnTime))
		{
			std::random_device rand;
			std::mt19937 gen(rand());

			Entity coinEntity = Engine::getCoordinator()->CreateEntity();

			Transform coinTransform;
			std::uniform_int_distribution<> randPos(0, 400);
			coinTransform.position = Vector2(randPos(gen), randPos(gen));
			coinTransform.scale = Vector2(30, 30);
			Engine::getCoordinator()->AddComponent<Transform>(coinEntity, coinTransform);

			Renderable coinSprite;
			memcpy(coinSprite.name, "Circle", 6);
			coinSprite.nameLen = 6;
			coinSprite.sourceRect.x = 0;
			coinSprite.sourceRect.y = 0;
			coinSprite.sourceRect.w = 256;
			coinSprite.sourceRect.h = 256;
			coinSprite.color = m_CoinColor;
			Engine::getCoordinator()->AddComponent<Renderable>(coinEntity, coinSprite);

			CreateNetworkedEntity(coinEntity);

			m_CoinTimer = 0;
			m_Coins++;

			return;
		}

		m_CoinTimer++;
	}

	/*
	* Create an entity that needs to be replicated across the network,
	* assigning it an available network ID and adding it to the entity
	* and ID mappings.
	*
	* This assumes that the entity being provided here already exists
	* and has been given components to replicate.
	*
	* Adds this entity to the update queue to be replicated across the
	* network in the Update loop.
	*/
	int ServerSystem::CreateNetworkedEntity(Entity entity)
	{
		NetworkID newNetID = m_AvailableNetworkIDs.front();
		m_AvailableNetworkIDs.pop();

		m_EntityToNetworkID[entity] = newNetID;
		m_NetworkIDToEntity[newNetID] = entity;

		m_UpdatedEntities.push(std::make_pair(newNetID, NetUpdateType::CREATED));
		return newNetID;
	}

	/*
	* Remove the entity and its corresponding NetworkID from the
	* entity and NetworkID mappings and return the NetworkID that
	* was being used to the AvailableNetworkIDs.
	*
	* Adds this entity to the update queue to be removed across
	* all clients in the Update loop.
	*/
	void ServerSystem::DeleteNetworkedEntity(Entity entity)
	{
		NetworkID deletedNetID = m_EntityToNetworkID[entity];
		Engine::getCoordinator()->DestroyEntity(entity);
		m_UpdatedEntities.push(std::make_pair(deletedNetID, NetUpdateType::DESTROYED));
	}

	/*
	* Adds the given entity to the update queue so its changes get
	* communicated across the server. This literally does nothing
	* besides indicating that this entity is different locally and
	* needs to be updated across all clients
	*/
	void ServerSystem::UpdateNetworkedEntity(Entity entity)
	{
		NetworkID entNetID = m_EntityToNetworkID[entity];
		m_UpdatedEntities.push(std::make_pair(entNetID, NetUpdateType::UPDATED));
	}
}