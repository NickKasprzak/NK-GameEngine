#include "Vector.h"
#include <math.h>

namespace Funny
{
	bool Vector2::operator==(Vector2& other)
	{
		if (x == other.x &&
			y == other.y)
		{
			return true;
		}

		return false;
	}

	Vector2 Vector2::operator+(Vector2& other)
	{
		Vector2 result;
		result.x = x + other.x;
		result.y = y + other.y;
		return result;
	}

	void Vector2::operator+=(Vector2& other)
	{
		this->x += other.x;
		this->y += other.y;
	}

	Vector2 Vector2::operator-(Vector2& other)
	{
		Vector2 result;
		result.x = x - other.x;
		result.y = y - other.y;
		return result;
	}

	void Vector2::operator-=(Vector2& other)
	{
		this->x -= other.x;
		this->y -= other.y;
	}

	Vector2 Vector2::operator*(float scalar)
	{
		Vector2 result;
		result.x = x * scalar;
		result.y = y * scalar;
		return result;
	}

	void Vector2::operator*=(float scalar)
	{
		this->x *= scalar;
		this->y *= scalar;
	}

	Vector2 Vector2::operator/(float scalar)
	{
		Vector2 result;
		result.x = x / scalar;
		result.y = y / scalar;
		return result;
	}

	void Vector2::operator/=(float scalar)
	{
		this->x /= scalar;
		this->y /= scalar;
	}

	void Vector2::addScaledVector(Vector2 vector, float scalar)
	{
		Vector2 scaled = vector * scalar;
		this->x += scaled.x;
		this->y += scaled.y;
	}

	Vector2 Vector2::normalize()
	{
		Vector2 normalizedVec;
		float magnitude = this->magnitude();

		normalizedVec = (*this) * (1.0f / magnitude);
		return normalizedVec;
	}

	float Vector2::magnitude()
	{
		return sqrtf((x*x) + (y*y));
	}

	float Vector2::magnitudeSquared()
	{
		return (x * x) + (y * y);
	}

	Vector2 Vector2::componentProduct(Vector2 other)
	{
		return Vector2(this->x * other.x, this->y * other.y);
	}

	void Vector2::componentProductUpdate(Vector2 other)
	{
		this->x *= other.x;
		this->y *= other.y;
	}

	float Vector2::dotProduct(Vector2 other)
	{
		return (this->x * other.x) + (this->y * other.y);
	}

	float Vector2::getAngle(Vector2 other)
	{
		float dot = this->dotProduct(other);
		float aMag = magnitude();
		float bMag = other.magnitude();
		
		return acos(dot / (aMag * bMag));
	}

	float Vector2::crossProduct(Vector2 other)
	{
		return (this->x * other.y) - (this->y * other.x);
	}

	void Vector2::makeOrthonormalBasis(Vector2& B, Vector2& C)
	{
		this->normalize();
		//C = crossProduct(B); - only works with 3D
		C.normalize();
		//B = crossProduct(C); - once again only work with 3D
	}
}