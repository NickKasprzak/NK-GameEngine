#pragma once
#include <cstdint>

namespace Funny
{
	typedef std::uint32_t uint32;
	struct ColorRGBA
	{
	public:
		ColorRGBA()
		{
		}

		ColorRGBA(uint32 r, uint32 g, uint32 b, uint32 a)
		{
			this->r = r;
			this->g = g;
			this->b = b;
			this->a = a;
		}

		ColorRGBA operator*(float& rhs);
		void operator*=(float& rhs);

		uint32 r = 0;
		uint32 g = 0;
		uint32 b = 0;
		uint32 a = 255;
	};
}