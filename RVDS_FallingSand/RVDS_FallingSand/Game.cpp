#include "Game.h"
#include <iostream>
#include "ServiceLocator.h"
#include "CPUSandSimulation.h"

Game::Game()
    : m_window(nullptr), m_isRunning(true), m_BrushSize(1)
{
    m_window = new Window("RVDS - Falling Sand Simulator", 640, 480);
    if (!m_window->Init())
    {
        std::cerr << "Failed to initialize window." << std::endl;
        m_isRunning = false;
    }

    // Register the CPU-based sand simulation as the default
    ServiceLocator::RegisterSandSimulation(std::make_unique<CPUSandSimulation>(640, 480));
}

Game::~Game()
{
    delete m_window;
}

void Game::Run()
{
    // Main game loop
    while (m_isRunning)
    {
        ProcessInput();  // Process user inputs and events
        Update();        // Game logic update
        Render();        // Render the window
    }
}

void Game::ProcessInput()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)  // Window close event
        {
            m_isRunning = false;
        }
        if (e.type == SDL_KEYDOWN)
        {
            switch (e.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                m_isRunning = false;
                break;
            // Increase brush size
            case SDLK_UP:
                m_BrushSize = std::min(m_BrushSize + 1, 20);  // Cap the brush size at 10 for now
                break;
            // Decrease brush size
            case SDLK_DOWN:
                m_BrushSize = std::max(m_BrushSize - 1, 1);  // Minimum brush size of 1
                break;
            default:
                break;
            }
        }
    }

    // Query the current mouse state at every frame
    int mouseX, mouseY;
    Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);  // Get the current mouse state and coordinates

    // Check if the left mouse button is pressed
    if (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT))
    {
        // Convert mouse position to grid coordinates
        size_t gridX = static_cast<size_t>(mouseX);
        size_t gridY = static_cast<size_t>(mouseY);

        // Radius for the circular brush
        int radius = m_BrushSize / 2;

        // Iterate over a square bounding the circle, but only place particles inside the circle
        for (int dx = -radius; dx <= radius; ++dx)
        {
            for (int dy = -radius; dy <= radius; ++dy)
            {
                // Calculate the distance from the center (gridX, gridY)
                int distanceSquared = dx * dx + dy * dy;

                // If the point is inside the radius (circle equation: x² + y² <= r²)
                if (distanceSquared <= radius * radius)
                {
                    size_t brushX = gridX + dx;
                    size_t brushY = gridY + dy;

                    // Place a particle if within grid bounds
                    if (brushX < m_window->GetWidth() && brushY < m_window->GetHeight())
                    {
                        ServiceLocator::GetSandSimulator().PlaceParticle(brushX, brushY);
                    }
                }
            }
        }
    }
}

void Game::Update()
{
    // Game logic update can be implemented here
    ServiceLocator::GetSandSimulator().Update();
}

void Game::Render() const
{
    m_window->Clear();  // Clear window to a specific color

    ServiceLocator::GetSandSimulator().Render(m_window->GetSurface());

    m_window->Update(); // Render the updated content
}
