#include "ResourceManager.h"

#include "Engine.h"
#include "RenderSystem.h"

namespace Funny
{
	std::unordered_map<std::string, SDL_Texture*> ResourceManager::m_Textures;

	SDL_Texture* ResourceManager::loadSDLTexture(std::string filepath, std::string name)
	{
		SDL_Surface* tempSurface = IMG_Load(filepath.c_str());
		if (tempSurface == nullptr)
		{
			std::cout << "Unable to create surface from image at path " << filepath << std::endl;
			return nullptr;
		}
		SDL_SetColorKey(tempSurface, SDL_TRUE, SDL_MapRGB(tempSurface->format, 0x00, 0x00, 0x00));

		SDL_Renderer* renderTarget = Engine::getCoordinator()->GetSystem<RenderSystem>()->getWindow().getSDLRenderer();
		SDL_Texture* newTexture = SDL_CreateTextureFromSurface(renderTarget, tempSurface);
		if (newTexture == nullptr)
		{
			std::cout << "Unable to create texture from surface " << name << std::endl;
		}

		else
		{
			std::cout << "Created texture at path [" << filepath << "]" << std::endl;
			m_Textures[name] = newTexture;
		}

		SDL_FreeSurface(tempSurface);
		tempSurface = nullptr;
		return newTexture;
	}

	SDL_Texture* ResourceManager::getSDLTexture(std::string name)
	{
		SDL_Texture* texture = m_Textures[name];

		if (texture == nullptr)
		{
			std::cout << "Texture named " << name << " does not exist." << std::endl;
		}

		return texture;
	}

	void ResourceManager::unloadTexture(std::string name)
	{
		SDL_Texture* target = getSDLTexture(name);

		if (target != nullptr)
		{
			std::cout << "Unloaded texture " << name << std::endl;
			SDL_DestroyTexture(target);
		}
	}

	void ResourceManager::unloadAllTextures()
	{
		for (auto i = m_Textures.begin(); i != m_Textures.end(); i++)
		{
			unloadTexture(i->first);
		}
	}
}