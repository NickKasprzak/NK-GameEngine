#pragma once
#include <SDL2/SDL.h>
#include "Common.h"

namespace Funny
{
	class Window
	{
	public:
		bool setMode(std::string name, int width, int height);
		bool isInit();
		bool close();

		SDL_Window* getSDLWindow() { return m_Window; }
		SDL_Renderer* getSDLRenderer() { return m_Renderer; };

	private:
		SDL_Window* m_Window = nullptr;
		SDL_Renderer* m_Renderer = nullptr;

		std::string m_Name;
		int m_Width;
		int m_Height;

		bool init();
	};
}