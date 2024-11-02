#include "Window.h"
#include <iostream> // For std::cerr

Window::Window(const std::string& title, int width, int height)
    : m_Title(title), m_Width(width), m_Height(height), m_pWindow(nullptr), m_pRenderer(nullptr) {}

Window::~Window() {
    // Clean up SDL resources
    SDL_DestroyRenderer(m_pRenderer);
    SDL_DestroyWindow(m_pWindow);
    SDL_Quit(); // Quit SDL subsystems
}

bool Window::Init() {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create SDL window
    m_pWindow = SDL_CreateWindow(m_Title.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        m_Width, m_Height, SDL_WINDOW_SHOWN);
    if (!m_pWindow) {
        std::cerr << "Window could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    // Create SDL renderer
    m_pRenderer = SDL_CreateRenderer(m_pWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!m_pRenderer) {
        std::cerr << "Renderer could not be created! SDL_Error: " << SDL_GetError() << std::endl;
        return false;
    }

    return true;
}

void Window::Clear() const {
    SDL_SetRenderDrawColor(m_pRenderer, 26, 26, 26, 255); // Set dark grey color
    SDL_RenderClear(m_pRenderer); // Clear the renderer with the set color
}

void Window::Update() const {
    SDL_RenderPresent(m_pRenderer); // Present the drawn frame
}