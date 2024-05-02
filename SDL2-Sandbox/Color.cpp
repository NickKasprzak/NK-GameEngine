#include "Color.h"

namespace Funny
{
	ColorRGBA ColorRGBA::operator*(float& rhs)
	{
		uint32 r = this->r * rhs;
		uint32 g = this->g * rhs;
		uint32 b = this->b * rhs;
		return ColorRGBA(r, g, b, this->a);
	}

	void ColorRGBA::operator*=(float& rhs)
	{
		r = r * rhs;
		g = g * rhs;
		b = b * rhs;
	}

	bool ColorRGBA::operator==(ColorRGBA& rhs)
	{
		if (r == rhs.r &&
			g == rhs.g &&
			b == rhs.b &&
			a == rhs.a)
		{
			return true;
		}

		return false;
	}
}