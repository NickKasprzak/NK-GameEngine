#pragma once
#include <cstdint>
#include <bitset>

#include "SDL2/SDL.h"
#include "Vector.h"
#include "Color.h"

namespace Funny
{
	/*
	* Defines our entities as just an integer
	* value. An Entity in an ECS is just an ID
	* value thats used to link different components
	* together that have been assigned to any
	* given Entity. When an Entity is created,
	* we're essentially just allowing for an
	* ID to have components assigned to it.
	* 
	* When simulating the game, all the components
	* attached to the given ID are updated
	* individually within all the systems that
	* require them, coming together in the
	* end to form unique Entities based on
	* their unique combinations and results
	* from our systems.
	* 
	* MAX_ENTITIES is assigned as an Entity
	* for the sake of keeping consistent
	* integer sizes for arrays n such.
	*/
	typedef std::uint16_t Entity;
	const Entity MAX_ENTITIES = 1000;

	/*
	* When a new Component is registered in
	* the Component manager, its assigned a
	* ComponentType ID. This ID is fully
	* unique to the given Component and
	* makes keeping track of what Entity
	* is using a given type a little simpler.
	*/
	typedef std::uint8_t ComponentType;
	const ComponentType MAX_COMPONENTS = 32;

	/*
	* A signature is a bitfield assigned to
	* every Entity thats created in order to
	* keep track of what components its using.
	* When a component is added or removed
	* from an Entity, the bit at the index
	* corresponding to the ComponentType is
	* flipped to true or false depending on
	* if its been added or removed.
	* 
	* This allows for O(1) lookups to see if
	* an Entity is using a component or not,
	* used by our Systems and SystemManager
	* to see if an Entity should be included
	* in its update process.
	*/
	typedef std::bitset<MAX_COMPONENTS> Signature;

	struct Renderable
	{
		SDL_Texture* texture;
		SDL_Rect sourceRect;
		ColorRGBA color
		{
			color.r = 255,
			color.g = 255,
			color.b = 255,
			color.a = 255
		};
		bool drawToScreen;
	};

	struct Transform
	{
		Vector2 position;
		Vector2 scale;
		Vector2 rotation;
	};
}