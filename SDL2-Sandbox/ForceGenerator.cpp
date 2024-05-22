#pragma once
#include "ForceGenerator.h"

namespace Funny
{
	void ParticleForceRegistery::add(Particle* particle, IForceGenerator* forceGen)
	{
		ParticleForceRegistration newRegi;
		newRegi.particle = particle;
		newRegi.forceGen = forceGen;
		registrations.push_back(newRegi);
	}

	void ParticleForceRegistery::remove(Particle* particle, IForceGenerator* forceGen)
	{
		// no idea how to make this not shit lol
	}

	void ParticleForceRegistery::clear()
	{
		registrations.clear();
	}

	void ParticleForceRegistery::updateForces(float dt)
	{
		for (auto const regi : registrations)
		{
			regi.forceGen->generateForce(regi.particle, dt);
		}
	}

	ParticleGravity::ParticleGravity(Vector2 grav)
	{
		gravity = grav;
	}

	void ParticleGravity::generateForce(Particle* particle, float dt)
	{
		// Break early is mass is unusable
		if (!particle->isFiniteMass()) { return; }

		/*
		* Add gravity scaled by the particle's mass. This
		* is done to negate the influence of the particle's
		* inverse mass during the integration step since
		* gravity isn't influenced by mass.
		*/
		particle->addForce(gravity * particle->getMass());
	}

	ParticleDrag::ParticleDrag(float k1, float k2)
	{
		this->k1 = k1;
		this->k2 = k2;
	}

	void ParticleDrag::generateForce(Particle* particle, float dt)
	{
		float drag = particle->getVelocity().magnitude();
		drag = (k1 * drag) * (k2 * drag * drag);

		Vector2 force = particle->getVelocity().normalize() * -drag;
		particle->addForce(force);
	}
}