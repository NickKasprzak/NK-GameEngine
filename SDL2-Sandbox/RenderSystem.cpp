#include "RenderSystem.h"

#include "Engine.h"
#include "Types.h"

namespace Funny
{
	RenderSystem::RenderSystem()
	{
		
	}

	RenderSystem::~RenderSystem()
	{
		m_Window.close();
	}

	void RenderSystem::Update()
	{
		/*
		* Loop through each of our entities and get their
		* renderable and transform components. They should
		* ALWAYS have both components as the RenderSystem
		* will have a signature that contains boths IDs.
		* 
		* Then create source and destination rects from this
		* data and pass it to the DrawSDLTexture function.
		* 
		* Later on, have positioning be done relative to
		* the camera's position rather than the screen.
		*/
		for (auto const& entity : m_ManagedEntities)
		{
			DrawEntity(entity);
		}

		// Have this draw all existing tilemaps rather
		// than just one.
		//DrawTilemap(Engine::test);
	}

	void RenderSystem::DrawEntity(Entity entity)
	{
		Transform entTrans = Engine::getCoordinator()->GetComponent<Transform>(entity);
		Renderable entRend = Engine::getCoordinator()->GetComponent<Renderable>(entity);

		SDL_Rect src = entRend.sourceRect;
		SDL_Rect dst
		{
			dst.x = entTrans.position.x,
			dst.y = entTrans.position.y,
			dst.w = entTrans.scale.x,
			dst.h = entTrans.scale.y
		};

		SDL_SetTextureColorMod(entRend.texture, entRend.color.r, entRend.color.g, entRend.color.b);
		SDL_SetTextureAlphaMod(entRend.texture, entRend.color.a);

		DrawSDLTexture(entRend.texture, src, dst);

		SDL_SetTextureColorMod(entRend.texture, 255, 255, 255);
		SDL_SetTextureAlphaMod(entRend.texture, 255);
	}

	/*
	* Only thing left to do now is get this to only
	* draw tiles that are in range of the camera.
	* We can probably do that by getting the closest
	* grid tile to the position of the camera's upper
	* and lower bounds horizontally and vertically and
	* only iterating across those tiles when drawing.
	* 
	* I can't think of a way to do this without a nested
	* for loop, one for the rows and one for the columns,
	* but it should be a net gain on performance when
	* dealing with larger maps.
	* 
	* Another issue is making sure we don't accidentally
	* step out of bounds of the tilegrid when attempting
	* to access a position that isn't actually occupied
	* by a valid space in the grid, instead having it
	* jump to the nearest tile actually in the grid thats
	* also in accordance with the XY position we're attempting
	* to access a tile at (so if we attempt to access a
	* tile with an Y world pos of 15, we should always get
	* a tile back that shares a world pos within that 15
	* Y pos range, no jumping to tiles lower/higher than
	* that by accident of using the tile's index rather
	* than nearest XY position).
	*/
	void RenderSystem::DrawTilemap(Tilemap* tilemap)
	{
		TilemapRenderData renderData = tilemap->GetRenderData();
		TilemapGridData gridData = tilemap->GetGridData();
		int tiles = tilemap->GetActiveTileCount();

		for (int i = 0; i < tiles; i++)
		{
			int spriteID = tilemap->GetTileSprite(i).spriteID;
			SDL_Rect src
			{
				src.x = (spriteID % renderData.xBounds),
				src.y = ((spriteID - src.x) / renderData.xBounds),
				src.w = renderData.tileWidth,
				src.h = renderData.tileHeight
			};
			src.x *= renderData.tileWidth;
			src.y *= renderData.tileHeight;

			Vector2 gridPos = tilemap->IndexToGrid(i);
			SDL_Rect dst
			{
				dst.x = gridData.transform.position.x + (gridPos.x * gridData.transform.scale.x),
				dst.y = gridData.transform.position.y + (gridPos.y * gridData.transform.scale.y),
				dst.w = gridData.transform.scale.x,
				dst.h = gridData.transform.scale.y
			};

			DrawSDLTexture(renderData.texture, src, dst);
		}
	}

	void RenderSystem::DrawSDLTexture(SDL_Texture* texture, SDL_Rect srcRect, SDL_Rect dstRect, bool drawToWorld)
	{
		if (texture == nullptr) { return; }
		SDL_Renderer* renderTarget = m_Window.getSDLRenderer();
		SDL_RenderCopy(renderTarget, texture, &srcRect, &dstRect);
	}

	void RenderSystem::RenderClear()
	{
		SDL_RenderClear(m_Window.getSDLRenderer());
	}

	void RenderSystem::RenderPresent()
	{
		SDL_RenderPresent(m_Window.getSDLRenderer());
	}
}