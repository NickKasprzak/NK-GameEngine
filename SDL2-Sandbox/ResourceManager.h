#pragma once
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "Common.h"

namespace Funny
{
	class ResourceManager
	{
	public:
		static SDL_Texture* loadSDLTexture(std::string filepath, std::string name);
		static SDL_Texture* getSDLTexture(std::string name);
		static void unloadTexture(std::string name);
		static void unloadAllTextures();

		static void loadPrimitives();

	private:
		static std::unordered_map<std::string, SDL_Texture*> m_Textures;
	};
}
