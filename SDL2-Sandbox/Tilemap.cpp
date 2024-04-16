#include "Tilemap.h"
#include "ResourceManager.h"
#include <cassert>

namespace Funny
{
	void Tilemap::loadMap()
	{
		m_GridData.xBounds = 10;
		m_GridData.yBounds = 5;
		m_GridData.tileCount = m_GridData.xBounds * m_GridData.yBounds;
		m_GridData.transform.position = Vector2(0, 0);
		m_GridData.transform.scale = Vector2(50, 50);

		m_RenderData.texture = ResourceManager::getSDLTexture("CaveTileset");
		m_RenderData.xBounds = 16;
		m_RenderData.yBounds = 5;
		m_RenderData.tileWidth = 16;
		m_RenderData.tileHeight = 16;
		m_RenderData.tileCount = m_RenderData.xBounds * m_RenderData.yBounds;

		std::string test = "2222222222222222222222222";

		for (int i = 0; i < m_GridData.tileCount; i++)
		{
			AddTile(TileIDToGrid(i), i);
		}
	}

	void Tilemap::unloadMap()
	{

	}

	void Tilemap::AddTile(Vector2 gridPos, int spriteID)
	{
		int tileID = GridToTileID(gridPos);
		int tileIndex = m_ActiveTileCount;

		if (spriteID > m_RenderData.tileCount)
		{
			std::cout << "Given sprite ID of " << spriteID << " exceeds total sprite count!" << std::endl;
			spriteID = 0;
		}

		m_Sprite[tileIndex].spriteID = spriteID;
		//m_Collision[tileIndex].collision = collisionType;

		m_TileIDToIndex[tileID] = tileIndex;
		m_IndexToTileID[tileIndex] = tileID;

		m_ActiveTileCount++;
	}

	void Tilemap::RemoveTile(Vector2 gridPos)
	{
		int removedTileID = GridToTileID(gridPos);
		int removedTileIndex = m_TileIDToIndex[removedTileID];

		int lastTileID = m_IndexToTileID[m_ActiveTileCount - 1];
		int lastTileIndex = m_ActiveTileCount - 1;

		/* 
		* Remove target tile by overwriting its data with
		* the last tile's data
		*/
		m_Sprite[removedTileIndex] = m_Sprite[lastTileIndex];
		// do same w/ collision

		/*
		* Have the ID of the last tile now point towards
		* the index of the tile it overwrote
		*/
		m_TileIDToIndex[lastTileID] = removedTileIndex;

		/*
		* Have the index of the overwritten tile now point
		* towards the ID of the last tile
		*/
		m_IndexToTileID[removedTileIndex] = lastTileID;

		/*
		* Remove the entries for now irrelevant data in the
		* maps (IndexToTileID for last tile and TileIDToIndex
		* for removed tile)
		*/
		m_TileIDToIndex.erase(removedTileID);
		m_IndexToTileID.erase(lastTileIndex);

		/*
		* Decrement total tile count by 1.
		*/
		m_ActiveTileCount--;
	}

	int Tilemap::GridToTileID(Vector2 gridPos)
	{
		assert((gridPos.x >= 0 && gridPos.x < m_GridData.xBounds) && "Given X position is out of bounds!");
		assert((gridPos.y >= 0 && gridPos.y < m_GridData.yBounds) && "Given Y position is out of bounds!");

		int tileID = 0;
		tileID += gridPos.y * m_GridData.xBounds;
		tileID += gridPos.x;

		assert((tileID >= 0 && tileID < m_GridData.tileCount) && "Calculated tile ID is out of bounds!");

		return tileID;
	}

	Vector2 Tilemap::TileIDToGrid(int tileID)
	{
		assert((tileID >= 0 && tileID < m_GridData.tileCount) && "Given tile ID is out of bounds!");

		int tileX = tileID % m_GridData.xBounds;
		int tileY = (tileID - tileX) / m_GridData.xBounds;

		assert((tileX >= 0 && tileX < m_GridData.xBounds) && "Calculated X position is out of bounds!");
		assert((tileY >= 0 && tileY < m_GridData.yBounds) && "Calculated Y position is out of bounds!");

		return Vector2(tileX, tileY);
	}

	Vector2 Tilemap::IndexToGrid(int index)
	{
		assert((index >= 0 && index < m_ActiveTileCount) && "Given tile index is out of bounds!");
		return TileIDToGrid(m_IndexToTileID[index]);
	}

	TileRenderable Tilemap::GetTileSprite(Vector2 gridPos)
	{
		int tileID = GridToTileID(gridPos);
		int tileIndex = m_TileIDToIndex[tileID];
		return m_Sprite[tileIndex];
	}

	TileRenderable Tilemap::GetTileSprite(int tileIndex)
	{
		return m_Sprite[tileIndex];
	}
}