#pragma once
#include <SDL2/SDL.h>
#include "System.hpp"
#include "Window.h"
#include "Tilemap.h"

namespace Funny
{
	// Rework this, its a little cumbersome to have to directly
	// update the dst pos each time we need to change where the
	// object renders when a component could contain info for
	// world position and the like.

	// Given that our components can access the entity it's attached
	// to, we could have a render system that iterates over entities
	// with "Renderable" components, a component that contains a sprite
	// and other relevant info, and adjust how the renderable is displayed
	// based on the other components the parent entity contains. The
	// render system itself handles the actual drawing of the SDL sprites
	// and handles things relative to the viewport, like camera and window.
	// This also means that Sprite will just be a wrapper for the SDL
	// texture and wont need a draw function, our render system will handle
	// drawing as it iterates over all renderable components.

	// The aforementioned renderable component can be adjusted by a
	// transform component, altering the renderable's draw position,
	// scale, and rotation through changing the destination rect. The 
	// animation component will also handle what aspects of the texture
	// will be drawn on screen through changing the source rect.

	// That all being said, I'm struggling to figure out what actually
	// needs to be stored in this sprite class since it doesn't really
	// do much beyond store a pointer to an SDL Texture. Looking at
	// Unity's documentation, their Sprite class only stores info
	// about how the sprite's texture is saved and such, which looks
	// to just be what our SDL Texture holds on to, so abstracting
	// everything about the SDL texture behind a sprite class when
	// it only offers stuff I don't need to worry much about seems
	// like it wouldn't be a good use of time. I think throwing it
	// directly into the Renderable component would be a better choice
	// as to not bloat the engine.

	// We also need to make sure that only objects within the camera's
	// field of view are rendered. No need to draw things we cant see.

	// Also, be sure to assert/handle situations where the rendering
	// system attempts to draw a texture that either doesn't exist or
	// is a nullptr.

	// I'm also a little torn on how I should be handling the components
	// of an entity from the rendering system, since having to check
	// EVERY entity the rendering system manages for an animation component
	// and a transform wouldn't be a clean or efficient solution. 
	// It would probably be better to handle animations from a different
	// system entirely that updates what part of the sprite the renderable
	// component samples in its update loop, incrementing timers and
	// swapping between src rects in accordance with how an animation
	// should be played. How that stuff is going to be serialized will
	// be a different story lol.

	// How should we handle rendering tilemaps too? Each tile isn't
	// going to be an entity with its own renderable and transform
	// since that'd be a waste of entity and component slots. The
	// tilemap itself is going to just be a bunch of data indicating
	// what each grid tile is going to contain as far as collision
	// info and what part of the grid's sprite grid it will sample
	// from when being drawn. We can handle drawing the tilemap
	// differently from a regular Renderable object, doing draw calls
	// based on tile info rather than Entity info. I can't have a
	// seperate system for drawing tilemaps and I can't manage different
	// sets of entities with unique signatures because that would
	// overcomplicate the process (managing different sets of components
	// with different functionalities and having different conditionals
	// for if they can be maintained by a system would be too much
	// and would interfere with the SystemManager, which we REALLY
	// dont want). My best guess would be to give our tilemap a transform
	// and renderable component so it gets passed into the rendering
	// system, but the rendering system does a different draw process
	// depending on if an entity also contains a tilemap component.
	// Only issue now is dealing with actually rendering things in
	// accordance how the rest of the system operates. I guess I
	// could have a function in the render system that takes arguments
	// for SDL texture, its source and destination rectangle, its
	// transform, and an enum(?) to decide if this transform should
	// be interpreted as world space or screen space. Only issue
	// with this solution is that we still check for a tilemap component
	// for every entity, which I don't think we want to have.

	// Some alternatives I found were to just have tilemaps be handled
	// seperately from the ECS entirely, reasoning being that it isn't
	// condusive to the design of the ECS and it doesn't require using
	// inheritence to function, which is what our ECS aims to avoid.
	// An ECS based engine doesn't have to work entirely within the
	// confines of the ECS, other systems can co-exist and function
	// alongside our ECS.

	class RenderSystem : public System
	{
	public:
		RenderSystem();
		~RenderSystem();

		void Update() override;
		void DrawEntity(Entity entity);
		void DrawTilemap(Tilemap* tilemap);

		void RenderClear();
		void RenderPresent();

		Window& getWindow() { return m_Window; };

	private:
		Window m_Window;

		void DrawSDLTexture(SDL_Texture* texture, SDL_Rect srcRect, SDL_Rect dstRect, bool drawToWorld = true);
	};
}