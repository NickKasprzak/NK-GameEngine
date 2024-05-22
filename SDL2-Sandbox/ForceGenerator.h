#pragma once
#include <vector>
#include "Particle.h"

namespace Funny
{
	/*
	* Force generators are a general interface for
	* any and all methods of generating a force to
	* be applied to our particles. Forces applied to
	* our particle can vary greatly within our engine,
	* being as simple as a one frame explosion or as
	* complex as a constant force applied for bouyancy
	* or a jetpack that changes its force applied
	* if the player is holding the action button or not.
	*
	* Since the way in which our force is generated
	* can vary on a type by type basis, its ideal 
	* that we're capable of seperating the force generation
	* specifics from our particle as much as possible.
	* Our interface class lets us grab the force a
	* force generator applies and accumulate it from
	* our particle, with the specifics of this force
	* generation being extended from a class that inherits
	* from the interface. 
	* 
	* ==== MY INTERJECTION ====
	* 
	* This would probably do wonders as a component.
	* Depending on how our integrator works, it could
	* be a system that processes the force generators
	* attached to an entity. We could still have objects
	* that exist as just force generators by attaching
	* one to an otherwise blank entity. The only thing
	* stopping this from happening is that we can't have
	* a component array of an interface type due to their
	* varying size in memory. The whole purpose of the
	* interface is to create more force types without
	* having to touch our physics engine, so we might
	* be able to achieve something similar through
	* scripting the unique force generation types to
	* suit our needs, applying the forces to our particle
	* there instead. We can even apply forces from the
	* script to the particle using the particle's addForce
	* method, abstracting out the rest of our physics
	* code assuming that the script is attached to an
	* entity that has a particle component. Either that
	* or we only create a handful of specific force types
	* for out-of-the-box use by the user and leave it to
	* them to create more force generators through scripts.
	* Things like springs, explosions, basic impulses, etc.
	* could be defined in-engine, while other gameplay
	* specific things would be defined through script.
	* This does appear to be gameplay level code afterall.
	* I guess I'll cross that bridge when I get to it lol.
	* 
	* Anyways, we give our force generator a virtual function
	* used to ensure that all our generators have some way
	* to create a force. This applies a force to our particle
	* for a duration of the frame, dt, and can be used
	* with any particle type so long as the generator doesn't
	* need to reference anything specific to a particle type.
	*/
	class IForceGenerator
	{
	public:
		virtual void generateForce(Particle* particle, float dt) = 0;
	};

	/*
	* Given our particles and force generators, we need
	* a fast and efficient way to handle applying forces
	* to only the particles that can be influenced by a
	* specific force type. We do this by managing the
	* pairing of each particle and the force generators
	* that can influence them, iterating through them
	* one by one and applying the forces accordingly.
	* 
	* ==== MY INTERJECTION ====
	* 
	* This is eerily similar to the ECS' way of handling
	* system and entity pairings, only difference being
	* that two objects are being paired for processing
	* to influence each other rather than a set of objects
	* being linked to be processed by one system. Given
	* that this is still just a system at its core (ie.
	* data being bound together to then be processed by
	* a system), this could ideally be integrated into the
	* System class structure. Each force generator could
	* be stored seperately as their specific force type
	* in their respective ComponentArrays (Explosion,
	* Spring, Impluse, etc.), but all derive from the
	* same ForceGenerator interface. Our physics system
	* would manage all of the entities that will be
	* affected by physics at all in the System class'
	* ManagedEntities array, while also managing two
	* seperate lists of ForceGenerators and the specific
	* signatures they need an Entity to have in order
	* to process them. That way, when we register an
	* Entity from the SystemManager, we can assign it
	* both to the system and to a specific force generator
	* type for processing. Granted, this is working off
	* of very limited knowledge as far as how the whole
	* system works, so this might not actually work in
	* execution based on the book's implementation.
	* Again, I'll cross that bridge when I get to it.
	*/
	class ParticleForceRegistery
	{
	public:
		/*
		* Adds a particle and force generator
		* pairing to the registry.
		*/
		void add(Particle* particle, IForceGenerator* forceGen);

		/*
		* Removes a particle and force generator
		* pairing from the registry.
		*/
		void remove(Particle* particle, IForceGenerator* forceGen);

		/*
		* Clears all particle and force generator
		* pairings.
		*/
		void clear();

		/*
		* Updates all particles to be influenced
		* by their assigned force generators by
		* the given delta time.
		*/
		void updateForces(float dt);

	private:
		
		/*
		* Struct for managing the pairing of a
		* specfic particle and force instance.
		* 
		* ==== MY INTERJECTION ====
		* 
		* I really don't like this. Why do we
		* group a specific particle instance
		* to a specific force instance? Wouldn't
		* it be better to process the particles
		* by ForceGen type rather than ForceGen
		* instance? And why do we need to have
		* this distinction of only having certain
		* particles be influenced by certain
		* forces? Shouldn't all particles be
		* influenced by all forces? Is this a
		* gameplay level thing that can be
		* worked around via script too? Ie.
		* only addForce to a particle if its
		* tagged as something? I don't think
		* Goodblox is going to need something of
		* this degree of flexibility. We'll see
		* I guess...
		* 
		* Insertion/deletion is O(n) too. If
		* we had entity/force TYPE mappings
		* instead of entity/force INST mappings,
		* we could add and remove entities w/o
		* having to touch the specific force
		* generator instances.
		* 
		* Only problem with mapping particles by
		* force TYPE is that we'd need seperate
		* maps for allll of our force types,
		* which can be a pretty big waste of space
		* provided that we don't end up using a
		* specific force that often. The alternative
		* is to check all physics managed entities
		* one by one during integration to see if
		* each force generator influences it, which
		* would still require managing all of our
		* forces on a type basis via array. And I
		* STILL haven't found out why we need to
		* have a distinction for if a force can
		* or cant influence a specific particle.
		* WE'LL SEE I GUESS.
		* 
		* Still, thinking about it now, it might be
		* better to just abstract ALL of this stuff
		* behind the physics engine and a few physics
		* components, leaving the ECS out of this
		* entirely. The components the engine would
		* offer would just act as a generic entry
		* point into the rest of the engine, while
		* keeping the lower level management and
		* calculations seperated from the core engine.
		* I don't think our physics engine needs to
		* be concerned with any of our other entities.
		* So long as we can adjust a particle's
		* values, have a to generate forces, can
		* link a particle to an entity via component,
		* and can communicate the tangible results
		* of a physics calculation to the rest of
		* the program, we should be good.
		*/
		struct ParticleForceRegistration
		{
			Particle* particle;
			IForceGenerator* forceGen;
		};

		/*
		* Holds all particle/force registrations
		*/
		std::vector<ParticleForceRegistration> registrations;
	};

	/*
	* Since gravity is just a force, we can represent gravity
	* as a force generator instead of just being a special case
	* added during the integration phase. Since we only need one
	* generator for gravity, we can just register a single instance
	* of this to every particle that needs to be influenced by
	* gravity. This also gives us a location where we can easily
	* set our global constant for acceleration due to gravity.
	* Despite these upsides, the approach of directly applying
	* gravity and damping during the integration step is what is
	* used throughout the rest of the book, as that method is
	* much faster computationally. This leaves these two force
	* generators unused.
	*
	* ==== MY INTERJECTION ====
	*
	* I see why we're registering force generators on a per instance
	* basis rather than a per type basis, since we really only need
	* to have one type of gravity influencing our particles at one
	* time. But I'm still being thrown off by any non-global force,
	* like an explosion or contact force. These things are only going
	* to last for a frame or so, so why would we need to go through
	* the hassle of registering and deregistering them if their
	* influence is limited entirely to where they're local to? And
	* what about something like a jetpack? Thats only going to be
	* influencing the objects its been attached to. Script level
	* stuff. Should these force generators only be used for global
	* forces that need to be applied to all/most particles?
	*/
	class ParticleGravity : public IForceGenerator
	{
	public:
		ParticleGravity(Vector2 grav);
		virtual void generateForce(Particle* particle, float dt);

	private:
		Vector2 gravity;
	};

	/*
	* Like with gravity, we can also apply drag through a
	* force generator. Since a realistic drag model would be
	* far too complex for it to be calculated on a frame by
	* frame basis, we instead use a simplified model that
	* applies a force in the direction opposite to the
	* particle's direction of velocity with strength that
	* varies with the speed and square speed. The formula
	* is as follows:
	* 
	* f(drag) = -normalized(velocity) * ( (k1 * speed) + (k2 * speed^2) )
	* 
	* with k1 and k2 being constants called the drag
	* coefficients. The higher k2 is, the faster the particle
	* will move at higher speeds.
	*/
	class ParticleDrag : public IForceGenerator
	{
	public:
		ParticleDrag(float k1, float k2);
		virtual void generateForce(Particle* particle, float dt);

	private:
		float k1;
		float k2;
	};
}