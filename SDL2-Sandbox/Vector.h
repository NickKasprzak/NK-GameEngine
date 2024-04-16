#pragma once

namespace Funny
{
	struct Vector2
	{
	public:
		Vector2()
		{
			x = 0;
			y = 0;
		}

		Vector2(float x, float y)
		{
			this->x = x;
			this->y = y;
		}

		bool operator==(Vector2& other);

		Vector2 operator+(Vector2& other);
		void operator+=(Vector2& other);

		Vector2 operator-(Vector2& other);
		void operator-=(Vector2& other);

		Vector2 operator*(float scalar);
		void operator*=(float scalar);

		Vector2 operator/(float scalar);
		void operator/=(float scalar);

		/*
		* Gets the direction of the vector
		* normalized to the 0-1 range.
		*/
		Vector2 normalize();

		/*
		* Gets the length of the vector.
		*/
		float magnitude();

		/*
		* Returns the squared magnitude, used
		* for faster comparisons of two vector's
		* magnitudes by omitting the sqrt step.
		*/
		float magnitudeSquared();

		/*
		* Gets the component product of this
		* vector and another vector, which is
		* just each of the vector components
		* multiplied by each other to form a
		* new vector.
		* 
		* This product has no real geometric
		* representation, so it doesn't see
		* much use.
		*/
		Vector2 componentProduct(Vector2 other);
		void componentProductUpdate(Vector2 other);

		/*
		* Gets the dot product (also known as
		* the scalar product) of this vector
		* and another vector, which is the
		* sum of each vector's component's
		* products (so x1*x2 + y1*y2). It
		* can also be represented as the
		* product of the magnitudes of the
		* two vectors multiplied by cos(theta)
		* using the angle formed between them.
		* A lot can be inferred about the
		* vectors based on the dot product.
		* 
		* |A| * |B| * cos(theta)
		* 
		* The cos(angle) alone tells us what
		* direction the two vectors point in
		* relative to each other. Cos(angle)'s
		* value ranges between -1 to 1. A value
		* of 1 tells us that the two vectors
		* point in the exact same direction,
		* while a value of -1 tells us that the
		* two vectors point in the exact opposite
		* direction. A value of 0 tells us their
		* directions are othogonal/perpendicular.
		* From this, we can infer that values
		* less than 0 mean that the two vectors
		* point more away from each other while
		* values greater than 0 point more towards
		* each other, or their similarity.
		* 
		* Multiplying this cos(angle) value
		* by the magnitudes of the vectors show
		* us how strongly these two vectors are
		* similar or dissimilar in their respective
		* directions. Two vectors of a high
		* magnitude will produce a far greater
		* magnitude, whereas two vectors of a low
		* magnitude will produce a smaller
		* magnitude. Scaling these by the value
		* of the cos(angle) will show us if the
		* vectors how strongly the two vectors
		* point towards a similar or opposite
		* direction, or how much one vector
		* lies in the direction of another.
		* 
		* With physics programming, the dot
		* product lets us calculate the magnitude
		* of one vector in the direction of another,
		* provided that one of the vectors are
		* unit length/normalized.
		*/
		float dotProduct(Vector2 other);

		/*
		* Gets the cross product (also known as
		* the vector product) of this vector
		* and another vector, which essentially
		* just cross multiplies each of the
		* two vector's components and subtracts
		* to get a new vector. Like the dot
		* product, the cross product can also
		* be represented by an angle related
		* relationship of the sum of the two
		* vectors magnitudes multiplied by
		* sin(theta), with theta still being the
		* angle formed by the two vectors
		* 
		* Its worth noting that the cross product
		* isn't commutative, meaning that the
		* result of a cross b and b cross a will
		* not yield the same result. However,
		* a cross b and -b cross a will yield
		* the same result.
		* 
		* Given that one vector is a normalized
		* value and another isn't, the magnitude
		* of the resulting vector given by the
		* cross product will be representative
		* of the component of the non-normalized
		* vector that isn't pointing in a direction
		* present in the normalized vector.
		* 
		* The cross product will always give a
		* vector that is fully perpendicular to
		* both of the given vectors (being at
		* a 90 degree angle compared to both,
		* or orthogonal). We can use this to
		* generate a unit vector thats orthogonal
		* to two vectors by normalizing the cross
		* product of two vectors.
		* 
		* In 2D, the cross product only returns
		* a float instead of a vector since having
		* a vector thats perpendicular to two
		* other vectors requires a 3D space. In
		* 2D, that vector is always pointing
		* towards the screen, so even if it did
		* exist, it wouldn't be very useful
		*/
		float crossProduct(Vector2 other);

		/*
		* In some cases, we might want to make
		* a fully orthogonal grouping of three
		* vectors, where each vector is at a
		* right angle to the other two. This
		* involves starting with two non-parallel
		* vectors. One vector, Vector A, will
		* keep its direction but will be normalized
		* if it isn't already. The other vector,
		* Vector B, might need to have its
		* magnitude and direction altered so
		* its orthogonal to A. The third vector,
		* Vector C, will be created through the
		* cross product of vectors A and B.
		* 
		* The algorithm might go as such:
		* - Normalize vector A
		* - Find C through the cross product of A and B
		* - If C has a magnitude of 0, A and B are parallel
		*	and the orthonormal basis can't be made
		* - Otherwise, normalize C
		* - Now that C and A are orthogonal to each other,
		*   we can recalculate B using them to make sure
		*   its also orthogonal
		* 
		* The orthogonal basis is used for contact
		* detection and resolution later on.
		* 
		* Note that the algorithm described above
		* only works for a right handed coordinate
		* system. Getting one for a left handed
		* coordinate system is as simple as changing
		* the order of operations for the cross
		* products.
		*/
		//void makeOrthonormalBasis();

		float x = 0;
		float y = 0;
	};
}