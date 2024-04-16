#include "Window.h"
#include "Common.h"

namespace Funny
{
	bool Window::setMode(std::string windowName, int windowWidth, int windowHeight)
	{
		m_Name = windowName;
		m_Width = windowWidth;
		m_Height = windowHeight;

		if (isInit())
		{
			SDL_SetWindowSize(m_Window, windowWidth, windowHeight);
			return true;
		}

		else
		{
			return init();
		}
	}

	bool Window::isInit()
	{
		if (m_Window == nullptr || m_Renderer == nullptr)
		{
			return false;
		}

		return true;
	}

	// Would this break our resource manager by re-initializing our SDL renderer?
	// Shouldn't be cause for concern unless we decided to need multiple windows
	// since our resource manager's textures are bound to a single SDL renderer,
	// which is bound to our single SDL window. But we don't need multiple windows.
	bool Window::init()
	{
		if (m_Window != nullptr)
		{
			SDL_DestroyWindow(m_Window);
		}

		if (m_Renderer != nullptr)
		{
			SDL_DestroyRenderer(m_Renderer);
		}

		m_Window = SDL_CreateWindow(m_Name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_Width, m_Height, 0);
		if (m_Window == nullptr)
		{
			std::cout << "Could not create SDL window." << std::endl;
			return false;
		}

		m_Renderer = SDL_CreateRenderer(m_Window, -1, 0);
		if (m_Renderer == nullptr)
		{
			std::cout << "Could not create SDL renderer." << std::endl;
			return false;
		}

		return true;
	}

	bool Window::close()
	{
		if (m_Renderer != nullptr)
		{
			SDL_DestroyRenderer(m_Renderer);
			m_Renderer = nullptr;
		}

		if (m_Window != nullptr)
		{
			SDL_DestroyWindow(m_Window);
			m_Window = nullptr;
		}

		return true;
	}
}