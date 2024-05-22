#pragma once

#include "Vector.h"

namespace Funny
{
	/*
	* The most basic object that can be simulated
	* in a physics simulation. It has a position,
	* velocity, and acceleration, but no orientation.
	* This means that it doesn't face in any given
	* direction, so we only really care about the
	* direction in which it travels.
	*/
	struct Particle
	{
	public:
		/*
		* When integrating our particle, we use the
		* typical position with respect to velocity,
		* acceleration, and time formula with integrals.
		* 
		* p' = p + vt + 1/2( at^2 )
		* 
		* However, since our timestep is usually so
		* small (a single frame is 1/60th of a second,
		* or 0.0166 seconds, in a 60 FPS game), the
		* acceleration part of the equation ends up
		* being so small its negligible. It gets
		* ommitted here in favor of handling change
		* in velocity due to acceleration seperately.
		* 
		* Speaking of which, our velocity update is
		* as follows:
		* 
		* v' = vd^t + at
		* 
		* with d being the damping factor. d is
		* raised to a power of t to ensure that
		* its applied proportionally to our time,
		* to avoid the velocity being damped at
		* the same rate each frame regardless of
		* how long each frame is taking.
		* 
		* Its noted that the process of raising
		* one float to the power of another is
		* a slow process, so with games simulating
		* a lot of objects at a time it'd be best
		* to calculate d^t before the simulation
		* as a constant.
		*/
		void integrate(float dt);

		/*
		* Functions for adding forces to our particle
		* and clearing the forces that have been
		* accumulated by it.
		* 
		* Whenever we want to apply a force, we need
		* to add it to the particle's force accumulator.
		* This process must be done before the integration
		* phase if we want force application to have results
		* the same frame they're applied. This is done
		* using force generators.
		* 
		* We also need to clear the accumulated forces
		* at the end of the integration phase, meaning
		* our current force generators will only be active
		* for one integration cycle.
		*/
		void addForce(Vector2 force);
		void clearForces();

		/*
		* Accessor functions for setting and getting the
		* normal mass and inverse mass, making it so that
		* we don't have to continually do new calculations
		* around the inverse mass stored locally.
		*/
		void setMass(float mass);
		void setInverseMass(float inverseMass);
		float getMass();
		float getInverseMass();

		/*
		* Functions for setting other particle attributes
		*/
		void setPosition(Vector2 pos);
		void setVelocity(Vector2 vel);
		void setAcceleration(Vector2 accel);
		void setDamping(float damping);

		/*
		* Functions for getting other particle attributes
		*/
		Vector2 getPosition();
		Vector2 getVelocity();
		Vector2 getAcceleration();
		float getDamping();

		/*
		* Helper function to check if the mass of this
		* particle is valid for use in calculations
		*/
		bool isFiniteMass();

	private:
		/*
		* Holds the linear position of the particle
		* in world space.
		*/
		Vector2 position;

		/*
		* Holds the linear velocity of the particle
		* in world space. This should only be updated
		* by the application of force.
		* 
		* It probably can be directly changed, but it
		* would be a bit jarring when taking other aspects
		* of our physics simulation into account.
		*/
		Vector2 velocity;

		/*
		* Holds the linear acceleration of the particle
		* in world space. Can be used to set any constant
		* acceleration, like gravity.
		* 
		* Newton's second law states that the application
		* of a force on a particle alters its acceleration.
		* Since acceleration is the only thing that alters
		* our velocity and position, it indirectly implies
		* that we can only changes those two things through
		* use of the application of forces.
		* 
		* Because of this law, we treat velocity and position
		* differently from acceleration. Velocity and postion
		* track a value frame to frame and only change through
		* our integrator. By contrast, acceleration can be
		* different frame to frame, either by direct altercation
		* of the value or being changed from application of force.
		* We can change acceleration to be anything we want from
		* one moment to the next and our physics simulation will
		* look just fine.
		*/
		Vector2 acceleration;

		/*
		* An example of this direct altercation is through
		* applying gravity. Speaking of gravity, the force
		* due to gravity will ALWAYS be the same regardless
		* of the object's mass, meaning its a constant force
		* applied in same to all objects being influenced
		* by gravity in our simulation. In fact, force due
		* to gravity can exist on a fully object to object
		* basis to allow for more fine-tuned physics
		* interactions, which we do here.
		*/
		Vector2 gravity;

		/*
		* Given that acceleration due to force also relies
		* on the mass of a particle being present, we need
		* to find a good way to represent it here. While
		* we could just add a scalar value for mass here,
		* this can pose issues in calculations in the event
		* mass is set to zero, resulting in any non-zero
		* force producing infinite acceleration and cause
		* divide by zero errors, which we don't want.
		* 
		* Its also useful to simulate particles with infinite
		* mass, meaning that they cannot be moved no matter
		* how much force is applied to them.
		* 
		* Since we can't actually represent infinity, we
		* need to find an alternative that lets us simulate
		* infinity despite that and also prevents a mass
		* of zero from ever occuring. Given that we can
		* represent acceleration as 1/m * f, we can choose
		* to instead store the 1/m, otherwise known as the
		* inverse mass. This lets us represent infinite mass
		* as zero inverse mass (as plugging in zero for 1/m
		* in the formula for acceleration will give zero) and
		* prevent a mass of zero from ever occurring (as
		* we can't get an inverse mass of infinity, something
		* only possible with a mass of zero)
		* 
		* The above only works provided that we're assigning
		* and utilizing this value as the inverse and not
		* the actual mass. We need to be absolutely certain
		* we're updating this as the inverse and not the
		* actual mass.
		*/
		float inverseMass;

		/*
		* Newton's first law states that an object not
		* being acted on by any forces will continue to
		* move at a constant rate at its current rate of
		* motion. This never occurs in the real world as
		* theres always some form of drag present, so
		* objects will come to a stop eventually.
		* 
		* In game physics, we need to simulate drag
		* regardless of if forces are at play. This is
		* due to floating point inaccuracies potentially
		* causing our particles to speed up despite not
		* having any forces acting on them. We call this
		* simple form of drag damping.
		* 
		* Our damping value is a number ranging between
		* 0 and 1, representing what percentage of velocity
		* our particle will have after each physics update.
		* A damping value of 0 will cause the particle to
		* lose all velocity while a value of 1 will cause
		* the particle to retain all of it. The higher the
		* value, the more velocity retained.
		* 
		* Its advised to not use a damping value of 1 since
		* it wont solve the issue of floating point calculations
		* causing issues. A damping value of 0.999 or something
		* similar is reccomended instead to make a particle
		* look like it isn't slowing down while also avoiding
		* any issues with calculations.
		*/
		float damping;

		/*
		* When applying forces to our particles, D'Alembert's
		* principal implies that the sum of all forces applied
		* to a particle at a given moment can be applied to
		* the particle as one combined force, being the accumulated
		* force here.
		*/
		Vector2 forceAccumulator;
	};
}