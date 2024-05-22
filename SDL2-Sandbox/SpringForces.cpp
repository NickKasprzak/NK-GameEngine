#include "SpringForces.h"

namespace Funny
{
	ParticleSpringGenerator::ParticleSpringGenerator(Particle* other, float restingLength, float springConstant)
	{
		this->other = other;
		this->restingLength = restingLength;
		this->springConstant = springConstant;
	}

	void ParticleSpringGenerator::generateForce(Particle* particle, float dt)
	{
		/*
		* Get the distance vector between our
		* two particles
		*/
		Vector2 dist;
		dist = particle->getPosition() - other->getPosition();

		/*
		* Get difference in particle distances
		* and the resting spring length
		*/
		float diff = dist.magnitude() - restingLength;

		/*
		* Plug everything into our interpretation
		* of Hooke's law.
		* 
		* Out of order bc of operator overloading schenanigans
		*/
		Vector2 force = dist * (-springConstant) * diff;
		particle->addForce(force);
	}

	ParticleAnchoredSpring::ParticleAnchoredSpring(Vector2* anchoredPosition, float restingLength, float springConstant)
	{
		this->anchoredPosition = anchoredPosition;
		this->restingLength = restingLength;
		this->springConstant = springConstant;
	}

	void ParticleAnchoredSpring::generateForce(Particle* particle, float dt)
	{
		/*
		* Get the distance vector between our
		* particle and the anchored position
		*/
		Vector2 dist;
		dist = particle->getPosition();
		dist -= *anchoredPosition;

		/*
		* Get the difference in the distance of
		* our particle and anchor and the resting
		* spring length
		*/
		float diff = dist.magnitude() - restingLength;

		/*
		* Plug everything into our interpretation
		* of Hooke's law.
		*
		* Out of order bc of operator overloading schenanigans
		*/
		Vector2 force = dist * (-springConstant) * diff;
		particle->addForce(force);
	}

	ParticleBungee::ParticleBungee(Particle* other, float restingLength, float springConstant)
	{
		this->other = other;
		this->restingLength = restingLength;
		this->springConstant = springConstant;
	}

	void ParticleBungee::generateForce(Particle* particle, float dt)
	{
		/*
		* Get the distance vector between our
		* two particles
		*/
		Vector2 dist;
		dist = particle->getPosition() - other->getPosition();

		/*
		* Check if our two particles are pulling
		* our spring apart by comparing their
		* distance to the resting length. If the
		* distance is greater than that of the
		* resting length, the two particles must
		* be being pulled apart.
		* 
		* By that logic, we skip calculating a
		* force if the above isn't the case.
		*/
		if (dist.magnitude() <= restingLength) { return; }

		/*
		* Get difference in particle distances
		* and the resting spring length
		*/
		float diff = dist.magnitude() - restingLength;

		/*
		* Plug everything into our interpretation
		* of Hooke's law.
		*
		* Out of order bc of operator overloading schenanigans
		*/
		Vector2 force = dist * (-springConstant) * diff;
		particle->addForce(force);
	}

	ParticleBouyancy::ParticleBouyancy(float maxSubmersionDepth, float objVolume, float waterDensity, float waterHeight)
	{
		this->maxSubmersionDepth = maxSubmersionDepth;
		this->objVolume = objVolume;
		this->waterDensity = waterDensity;
		this->waterHeight = waterHeight;
	}

	void ParticleBouyancy::generateForce(Particle* particle, float dt)
	{
		/*
		* Get depth of particle relative to the water
		* level's height
		*/
		float depth = particle->getPosition().y - waterHeight;

		/*
		* Don't apply a bouyancy force if the particle
		* is above/out of the water
		*/
		if (depth >= maxSubmersionDepth) { return; }

		Vector2 force;

		/*
		* Add the highest amount of bouyancy force
		* immediately if our particle is below the
		* maximum submersion depth
		*/
		if (depth <= maxSubmersionDepth)
		{
			force.y = waterDensity * objVolume;
			particle->addForce(force);
			return;
		}

		/*
		* Otherwise, our particle is only partly
		* submerged, so the water displacement, and
		* by extension our force, is only part of
		* our volume's total submersion.
		*/
		force.y = waterDensity * objVolume;
		force.y *= ((depth - maxSubmersionDepth) / 2) * maxSubmersionDepth; // dont really get this
		particle->addForce(force);
	}
}