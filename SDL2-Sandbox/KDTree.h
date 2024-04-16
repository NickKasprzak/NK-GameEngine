#pragma once
#include "Vector.h"

namespace Funny
{
	/*
	* Right now, my goal with the KDTree is
	* to do quick range sorting and searches
	* between the two sets of ranges a biome
	* has (terrain height range and moisture
	* range). Provided a terrain height and
	* moisture level for a given pixel, I
	* need a fast way to first find the biomes
	* with a terrain height range that the
	* given terrain height value falls into,
	* then be able to find the biome with
	* the moisture range the given moisture
	* level falls into.
	* 
	* I figured a KDTree works well for this
	* considering it partitions things based
	* on a dimensional set of values, and the
	* terrain/moisture ranges are just a 2D
	* vector, so I feel it stands to reason
	* that our range values can be thought
	* of as a different form of space.
	* 
	* I'm only getting tripped up by collisions
	* on this one. A lot of examples have a
	* few repeat ranges on the tree, so
	* I'm unsure what the motive behind picking
	* one range over the other would be, or
	* how traversing the tree to find all
	* possible ranges and going from there.
	* 
	* I do need to figure out how to handle
	* collisions properly as multiple biomes
	* could exist within a certain range of
	* elevations. I need to be able to pull
	* out all of those biomes based on terrain,
	* then sort them in another KDTree based
	* on moisture. That way I get all the biomes
	* that exist at a certain elevation and can
	* then choose the one that also fits the
	* moisture level.
	* 
	* I realize now that collisions in both
	* moisture and elevation could pose problems.
	* I can't just randomly pick between them
	* as it'll just lead to a random assortment
	* of different biomes being generated within
	* the given location, which would break
	* coherency and look ugly.
	* 
	* It's possible I could just try to avoid
	* collisions all together, but it feels
	* like it'd be too hard to manage. Maybe
	* it'd be easier to manage if moisture
	* generation was done first, then a biome
	* was picked based on the moisture level,
	* and then terrain was generated using the
	* biome's terrain generation settings.
	* 
	* In that case though, would we also have
	* to account for the surrounding terrain's
	* generation when picking a biome? Or sould
	* moisture just pose a random possible list
	* of biomes we could pick from regardless of
	* the surrounding terrain?
	* 
	* This was so naive lol. Minecraft's generation
	* is way more complex than height and moisture
	* maps. So much more goes into it. Still unsure
	* about the order of operations though.
	*/
	class KDTree
	{
	public:

	private:

	};
}