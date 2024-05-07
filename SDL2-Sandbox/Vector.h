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
		* A simple helper function that adds
		* a vector of a scaled quantity to
		* this vector.
		* 
		* Can be used for things like integration
		* where we need to add a velocity vector
		* scaled by delta time to get the change
		* in position for a timestep.
		*/
		void addScaledVector(Vector2 vector, float scalar);

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
		* by the magnitudes of the vectors offers
		* a quantification of how much the component
		* of one vector points in the direction
		* of another, rather than just being with
		* respect to direction. Given vectors A and
		* B, the dot product would offer the component
		* of B thats pointing in the direction of A.
		* 
		* For example, the normalized Vector A has
		* a magnitude of 1 and Vector B has a magnitude
		* of 2. If Vector B is pointing off in an almost
		* perpendicular direction to A, despite having
		* a high magnitude, its dot product will be
		* a lower value like 0.3 since only some of its
		* strength is being directed towards where A is
		* pointing. This also applies with negative
		* dot product.
		* 
		* With physics programming, the dot
		* product lets us calculate the magnitude
		* of one vector in the direction of another,
		* provided that one of the vectors are
		* unit length/normalized.
		*/
		float dotProduct(Vector2 other);

		/*
		* Given the relationship between the dot
		* product and the angle formed between
		* two vectors, seen with:
		* 
		* a dot b = |a| |b| * cos(theta)
		* 
		* provided that a and b are normalized,
		* we can find the angle formed between
		* them using the following formula:
		* 
		* acos(a dot b) = theta
		* 
		* If they aren't normalized, then the
		* angle would have to be found using:
		* 
		* acos( (a dot b) / |a| |b| ) = theta
		*/
		float getAngle(Vector2 other);

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
		* right angle to the other two. We need
		* two non-parallel vectors to start with.
		* 
		* Our primary vector, Vector A, serves as
		* the basis for the orthonormal basis. It
		* is normalized if it hasn't already been.
		* 
		* Our secondary vector, Vector B, serves the
		* sole purpose of generating Vector C through
		* the cross product of it and A. It will have
		* its direction and magnitude changes to be
		* orthogonal to A and C if it isn't already
		* after the fact.
		* 
		* As mentioned above, our tertiary vector,
		* Vector C, is generated as the cross product
		* of Vector A and Vector B.
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
		* 
		* Note that an orthonormal basis can only
		* be generated for a 3D coordinate plane,
		* as it relies on the cross product to
		* generate results. Its only defined here
		* for reference purposes.
		*/
		void makeOrthonormalBasis(Vector2& B, Vector2& C);

		float x = 0;
		float y = 0;
	};
}