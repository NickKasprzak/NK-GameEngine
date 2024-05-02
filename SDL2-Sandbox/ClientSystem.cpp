#include "ClientSystem.h"
#include "Engine.h"
#include "PacketTypes.h"
#include "ResourceManager.h"

namespace Funny
{
	ClientSystem* ClientSystem::m_Instance = 0;

	void ClientSystem::init()
	{
		m_Socket.Create(Socket::Family::INET, Socket::Type::STREAM);
		m_Socket.Connect(Address("127.0.0.1", 2000));
		m_Socket.SetNonBlockingMode(true);
	}

	bool ClientSystem::update()
	{
		RecieveFromServer();

		// process input

		// send input packets

		SDL_Event e;
		while (SDL_PollEvent(&e) != 0)
		{
			if (e.type == SDL_QUIT)
			{
				return false;
			}

			else if (e.type == SDL_KEYDOWN)
			{
				PacketHeader hPack;
				hPack.packetType = PacketTypes::CLIENT_INPUT;

				ClientInputPacket iPack;

				switch (e.key.keysym.sym)
				{
					case SDLK_UP:
						iPack.inputDir = InputDirections::UP;
						break;

					case SDLK_DOWN:
						iPack.inputDir = InputDirections::DOWN;
						break;

					case SDLK_LEFT:
						iPack.inputDir = InputDirections::LEFT;
						break;

					case SDLK_RIGHT:
						iPack.inputDir = InputDirections::RIGHT;
						break;

					default:
						iPack.inputDir = InputDirections::NO_INPUT;
						break;
				}

				if (iPack.inputDir != InputDirections::NO_INPUT)
				{
					ByteStream stream;
					hPack.SerializePacket(stream);
					iPack.SerializePacket(stream);

					char sendBuff[4096];
					size_t bytes = 0;
					stream.getBuffer(sendBuff, 4096, bytes);

					m_Socket.Send(sendBuff, bytes);
				}
			}
		}

		return true;
	}

	void ClientSystem::RecieveFromServer()
	{
		char recvBuff[4096];
		int bytesRecv = m_Socket.Recv(recvBuff, 4096);

		if (bytesRecv == -1)
		{
			if (m_Socket.GetLastError() != Socket::Error::SOCKLIB_EWOULDBLOCK)
			{
				assert("Something went wrong recieving from server.");
			}
		}

		if (bytesRecv != -1)
		{
			ByteStream stream;
			stream.setBuffer(recvBuff, bytesRecv);
			
			while (ProcessPacket(stream))
			{

			}
		}
	}

	bool ClientSystem::ProcessPacket(ByteStream& stream)
	{
		if (stream.endOfStream()) { return false; }

		PacketHeader packHeader;
		packHeader.DeserializePacket(stream);

		if (packHeader.packetType == PacketTypes::ENTITY_UPDATE)
		{
			EntityPacketHeader entHeader;
			entHeader.DeserializePacket(stream);

			if (entHeader.updateType == NetUpdateType::CREATED)
			{
				Entity newEnt = Engine::getInstance()->getCoordinator()->CreateEntity();

				Signature tempSig;

				// Check for and deserialize transform
				ComponentType transformType = Engine::getCoordinator()->GetComponentType<Transform>();
				tempSig.flip(transformType);
				if ((entHeader.signature & tempSig) != 0)
				{
					TransformPacket tPack;
					tPack.DeserializePacket(stream);

					Transform transform
					{
						transform.position = tPack.position,
						transform.scale = tPack.scale
					};

					Engine::getInstance()->getCoordinator()->AddComponent<Transform>(newEnt, transform);
				}
				tempSig.reset();

				// Check for and deserialize renderable
				ComponentType renderableType = Engine::getCoordinator()->GetComponentType<Renderable>();
				tempSig.flip(renderableType);
				if ((entHeader.signature & tempSig) != 0)
				{
					RenderablePacket tRend;
					tRend.DeserializePacket(stream);

					std::string textureName(tRend.name, tRend.nameSize);
					Renderable renderable;
					renderable.texture = ResourceManager::getSDLTexture(textureName);
					renderable.sourceRect.x = tRend.sourceRect.x;
					renderable.sourceRect.y = tRend.sourceRect.y;
					renderable.sourceRect.w = tRend.sourceRect.w;
					renderable.sourceRect.h = tRend.sourceRect.h;
					renderable.color.r = tRend.color.r;
					renderable.color.g = tRend.color.g;
					renderable.color.b = tRend.color.b;
					renderable.color.a = tRend.color.a;
					Engine::getInstance()->getCoordinator()->AddComponent<Renderable>(newEnt, renderable);
				}
				tempSig.reset();

				m_EntityToNetworkID[newEnt] = entHeader.networkID;
				m_NetworkIDToEntity[entHeader.networkID] = newEnt;
			}

			// This ballooned in scope too much and im running out of time so now im just
			// having the game assume that the entities have a transform and renderable already.
			else if (entHeader.updateType == NetUpdateType::UPDATED)
			{
				TransformPacket tPack;
				tPack.DeserializePacket(stream);

				RenderablePacket tRend;
				tRend.DeserializePacket(stream);

				Entity ent = m_NetworkIDToEntity[entHeader.networkID];
				Transform& transform = Engine::getCoordinator()->GetComponent<Transform>(ent);
				transform.position = tPack.position;
			}

			else if (entHeader.updateType == NetUpdateType::DESTROYED)
			{
				Entity deletedEntity = m_NetworkIDToEntity[entHeader.networkID];
				Engine::getCoordinator()->DestroyEntity(deletedEntity);

				m_NetworkIDToEntity.erase(entHeader.networkID);
				m_EntityToNetworkID.erase(deletedEntity);
			}
		}

		else if (packHeader.packetType == PacketTypes::GAME_MESSAGE)
		{
			GameMessagePacket mPack;
			mPack.DeserializePacket(stream);

			// I dont think we even need this.
			char netIDMessage[16] = "Your NetID is: ";
			if (memcmp(mPack.message, netIDMessage, 15) == 0)
			{
				
			}
		}
		
		return true;
	}
}