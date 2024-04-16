#pragma once
#include <unordered_map>
#include <array>
#include <SDL2/SDL.h>
#include "Vector.h"
#include "Types.h"

namespace Funny
{
	/*
	* My current best solution is to have
	* this class manage two compact arrays
	* for tile data, one for collision data
	* and one for sprite data. Both would
	* be unique structs that contains info
	* specific to the processes relevant to
	* the set, making passing things to and
	* from cache a faster process that doesn't
	* involve throwing a lot of irrelevant
	* data to whatever system needs to operate
	* on it.
	* 
	* The arrays will be one dimensional for
	* the sake of keeping data compact and make
	* iterating over them a faster process.
	* Indexing into a tile will be done via
	* grid coordinates, converting the 2D
	* coordinates to 1D coordinates by multiplying
	* the X coordinate by the width of the
	* grid and adding the Y coordinate to that.
	* 
	* Since our array is going to be compact,
	* meaning that everything will remain
	* next to each other sequentially in memory,
	* indexing into or out of a tile will have
	* to be done via an unordered map that
	* maps the 1D coordinate of a tile to
	* the index in the compact array and
	* vice versa. This should allow for O(1)
	* lookups for a tile given a 2D coordinate,
	* as well as converting that tile back to
	* a 2D coordinate provided the tile.
	* 
	* The tilemap also needs a way to convert
	* a location in worldspace to a corresponding
	* tilemap grid coordinate. We can do this
	* by modularly dividing the given coordinate
	* by the width and height of the grids tile.
	* This should let us check if any space in
	* worldspace is occupied by a tile in O(1)
	* lookup time, as well as find a range of
	* tiles that need to be rendered based on the
	* camera's position and its respective viewport
	* size relative to that position.
	* 
	* The tile visual struct will contain data
	* corresponding to what part of its parent
	* tilemap it should sample from. I'm torn
	* on whether this should be either a basic
	* ID that corresponds to the part of the
	* tilemap it should sample, or it should
	* be the raw source rectangle data.
	* 
	* The former solution would involve calculating
	* a source rect based on the ID. Given a
	* tile sheet that is 16x16 pixels per tile,
	* 64 pixels wide and 32 pixels tall, it'd
	* have a total of 8 possible tile IDs, 4
	* across the top row of tiles and 4 across the
	* bottom row of tiles. Tile 0 would be at 0, 0,
	* tile 3 would be at 46, 0, and tile 5 would be
	* at 0, 16. Given that the sizes of our tiles
	* will remain constant, meaning each tile sprite
	* will be limited to the bounds of its corresponding
	* grid location, this implementation would help
	* in maintaining that fact by implicitly assuming
	* we're sampling tiles to always be the same
	* pixel size and won't have any disparities in
	* their resulting resolution.
	* 
	* While the latter would offer a nice amount of
	* flexibility as far as sampling different sprite
	* sizes, it wouldn't work well in the context of
	* our tilemap size we want our sprites to all share
	* the same resolution (no 16x16 tile should be placed
	* right next to a 32x32 tile for example).
	* 
	* The tile collision struct will only contain
	* basic information about the collision data of
	* the tile. This is simply just an enum that
	* says what kind of collision the tile should
	* have (ie. NONE, BLOCK, SLOPE, WATER, etc.)
	* which are then interpreted as their corresponding
	* collision types by the physics system when
	* handling collision.
	* 
	* I'm a little unsure as to how I'm going to
	* serialize all this and bridge the visual and
	* collision components, like having certain
	* sprites be paired with certain collision types.
	* I feel like it'd probably be best to handle
	* this stuff at the editor level rather than
	* engine level since thats where we're going to
	* be interfacing with the tilemap anyways to
	* make levels. Could probably just save all the
	* tilemap data in a binary format.
	* 
	* Another fun little bonus I just realized about
	* having a 1D array to store tile data is that we
	* don't have to worry about dynamically allocating
	* a 2D vector/array of tiles at runtime based on
	* the width and height we want our level to be
	* since the 1D array will always remain constant
	* size. This also means that we can just reuse
	* the same tilemap data structure between level
	* loads, overwriting old data with new data in
	* the arrays and reconstructing the index mapping
	* maps accordingly.
	*/
	const int MAX_TILES = 5000;

	enum TileCollisionType : uint8_t
	{
		NONE,
		BLOCK,
		SLOPE_LEFT,
		SLOPE_RIGHT,
		COLLISION_TYPES
	};

	/*
	* Details about how the sprite sheet
	* should be interpreted
	*/
	struct TilemapRenderData
	{
		SDL_Texture* texture;

		int xBounds;
		int yBounds;
		int tileWidth;
		int tileHeight;

		int tileCount;
	};

	/*
	* Details about how the tilemap's data
	* should be interpreted
	*/
	struct TilemapGridData
	{
		Transform transform;
		int xBounds;
		int yBounds;

		int tileCount;
	};

	struct TileRenderable
	{
		uint8_t spriteID;
	};

	struct TileCollision
	{
		TileCollisionType type;
	};

	class Tilemap
	{
	public:
		void loadMap();
		void unloadMap();

		void AddTile(Vector2 gridPos, int spriteID);
		void RemoveTile(Vector2 gridPos);

		int GridToTileID(Vector2 gridPos); // Converts from grid pos to tile ID
		Vector2 TileIDToGrid(int tileID); // Converts from tile ID to grid position
		Vector2 IndexToGrid(int index); // Converts from index to grid position
		int GetActiveTileCount() { return m_ActiveTileCount; }

		TilemapRenderData GetRenderData() { return m_RenderData; }
		TilemapGridData GetGridData() { return m_GridData; }

		TileRenderable GetTileSprite(Vector2 gridPos); // Gets tile sprite by grid position
		TileRenderable GetTileSprite(int tileIndex); // Gets tile sprite by index in tile array

		//TileCollision GetTileCollision(Vector2 gridPos);
		//TileCollision GetTileCollision(int tileIndex);

	private:
		TilemapRenderData m_RenderData;
		TilemapGridData m_GridData;

		int m_ActiveTileCount;

		// Actual data for our tiles

		std::array<TileRenderable, MAX_TILES> m_Sprite;
		std::array<TileCollision, MAX_TILES> m_Collision;

		std::unordered_map<int, int> m_IndexToTileID; // From an index in the array to the tile's one dimensional tile ID
		std::unordered_map<int, int> m_TileIDToIndex; // From the tile's one dimensional tile ID to its index in the array
	};
}