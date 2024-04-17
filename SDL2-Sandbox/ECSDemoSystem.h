#pragma once
#include "System.hpp"

namespace Funny
{
	class ECSDemoSystem : public System
	{
	public:
		ECSDemoSystem();
		~ECSDemoSystem();

		void Update() override;

	private:
		int m_GeneratedEntities = 0;
		int m_MaxGeneratedEntities = 1000;

		int m_XSpawnPosMin = 0;
		int m_XSpawnPosMax = 640;

		int m_YSpawnPos = 0;
		int m_YDespawnPos = 2000;
	};
}