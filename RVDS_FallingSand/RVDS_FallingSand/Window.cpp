#include "Window.h"
#include <iostream>

Window::Window(const std::string& title, int width, int height)
    : m_title(title), m_width(width), m_height(height), m_window(nullptr), m_surface(nullptr)
{
}

Window::~Window()
{
    SDL_DestroyWindow(m_window);
    SDL_Quit();
}

bool Window::Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    m_window = SDL_CreateWindow(m_title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_width, m_height, SDL_WINDOW_SHOWN);
    if (!m_window)
    {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    m_surface = SDL_GetWindowSurface(m_window);
    return true;
}

void Window::Clear() const
{
    SDL_FillRect(m_surface, nullptr, SDL_MapRGB(m_surface->format, 0xA1, 0xA3, 0xB5)); // Grayish background
}

void Window::Update() const
{
    SDL_UpdateWindowSurface(m_window);
}
