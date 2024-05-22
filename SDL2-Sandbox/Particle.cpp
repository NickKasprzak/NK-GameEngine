#include "Particle.h"
#include <cassert>
#include <cmath>

namespace Funny
{
	void Particle::integrate(float dt)
	{
		/*
		* Skip attempting to integrate a
		* particle with 0 inverse mass.
		*/
		if (inverseMass <= 0) { return; }

		// Can't have a timestep of 0
		assert(dt > 0 && "Can't simulate an object using no timestep!");

		// Update our position vector from velocity
		position.addScaledVector(velocity, dt);

		/*
		* Calculate the new acceleration for our particle
		* based on the forces that have been accumulated
		* by it. Since force application changes our
		* particle's acceleration, not directly altering
		* it, we just add the acceleration calculated
		* from a = (1 / m)F to our current acceleration.
		* 
		* We don't need to directly modify our acceleration
		* value here as acceleration due to force isn't
		* constant, it doesn't and shouldn't be applied
		* every frame. We only directly alter it for
		* things like gravity. Its impact will still be
		* felt by velocity beyond its initial application
		* frame and integration cycle.
		*/
		Vector2 resultingAccel = acceleration;
		resultingAccel.addScaledVector(forceAccumulator, inverseMass);

		// Update our velocity vector from acceleration
		velocity.addScaledVector(resultingAccel, dt);

		// Update our velocity vector through damping
		velocity *= powf(damping, dt);

		// Clear the forces applied to the particle
		// We touch on this later as well.
		clearForces();
	}

	void Particle::addForce(Vector2 force)
	{
		forceAccumulator += force;
	}

	void Particle::clearForces()
	{
		forceAccumulator = Vector2(0, 0);
	}

	void Particle::setMass(float mass)
	{
		if (mass == 0) { inverseMass = 0; return; }
		inverseMass = 1.0f / mass;
	}

	void Particle::setInverseMass(float inverseMass)
	{
		this->inverseMass = inverseMass;
	}

	float Particle::getMass()
	{
		if (inverseMass == 0) { return 0; }
		return 1.0f / inverseMass;
	}

	float Particle::getInverseMass()
	{
		return inverseMass;
	}

	void Particle::setPosition(Vector2 pos)
	{
		position = pos;
	}

	void Particle::setVelocity(Vector2 vel)
	{
		velocity = vel;
	}

	void Particle::setAcceleration(Vector2 accel)
	{
		acceleration = accel;
	}

	void Particle::setDamping(float damping)
	{
		this->damping = damping;
	}

	Vector2 Particle::getPosition()
	{
		return position;
	}

	Vector2 Particle::getVelocity()
	{
		return velocity;
	}

	Vector2 Particle::getAcceleration()
	{
		return acceleration;
	}

	float Particle::getDamping()
	{
		return damping;
	}

	bool Particle::isFiniteMass()
	{
		if (inverseMass > 0) { return true; }
		return false;
	}
}