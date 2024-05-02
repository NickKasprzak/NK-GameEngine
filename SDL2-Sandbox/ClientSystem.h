#pragma once
#include "SimpleSock/socklib.h"
#include "ByteStream.h"
#include "PacketTypes.h"
#include "Types.h"
#include <unordered_map>

namespace Funny
{
	class ClientSystem
	{
	public:
		static ClientSystem* createInstance()
		{
			if (m_Instance == nullptr)
			{
				m_Instance = new ClientSystem;
				return m_Instance;
			}

			return nullptr;
		}
		static ClientSystem* getInstance() { return m_Instance; }

		void init();
		bool update();
		void RecieveFromServer();
		bool ProcessPacket(ByteStream& stream);

	private:
		static ClientSystem* m_Instance;
		Socket m_Socket;

		std::unordered_map<Entity, NetworkID> m_EntityToNetworkID;
		std::unordered_map<NetworkID, Entity> m_NetworkIDToEntity;
	};
}