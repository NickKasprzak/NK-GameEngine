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
}