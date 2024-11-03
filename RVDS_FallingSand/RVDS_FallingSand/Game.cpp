#include "Game.h"
#include <iostream>
#include "ServiceLocator.h"
#include "CPUSandSimulation.h"
#include "InputManager.h"

Game::Game()
    : m_pWindow(nullptr), m_IsRunning(true), m_BrushSize(1)
{
    m_pWindow = new Window("RVDS - Falling Sand Simulator", 1280, 720);
    if (!m_pWindow->Init())
    {
        std::cerr << "Failed to initialize window." << std::endl;
        m_IsRunning = false;
    }

    // Register the CPU-based sand simulation as the default
    ServiceLocator::RegisterSandSimulation(std::make_unique<CPUSandSimulation>(GridInfo{ glm::ivec2{0, 20}, 48, 72, 10 }, m_pWindow));
}

Game::~Game()
{
    delete m_pWindow;
}

void Game::Run()
{
    // Main game loop
    while (m_IsRunning)
    {

        ProcessInput();  // Process user inputs and events

        // Calculate delta time
        auto currentTime = std::chrono::steady_clock::now();
        std::chrono::duration<float> elapsed = currentTime - m_LastTime;
        float deltaTime = elapsed.count();
        m_LastTime = currentTime;

        Update(deltaTime);  // Game logic update
        Render();           // Render the window
    }
}

void Game::ProcessInput()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_QUIT)  // Window close event
        {
            m_IsRunning = false;
        }
        if (e.type == SDL_KEYDOWN)
        {
            switch (e.key.keysym.sym)
            {
            case SDLK_ESCAPE:
                m_IsRunning = false;
                break;
            default:
                break;
            }
        }
    }

    InputManager::GetInstance().Update();

    //// Query the current mouse state at every frame
    //int mouseX, mouseY;
    //Uint32 mouseButtons = SDL_GetMouseState(&mouseX, &mouseY);  // Get the current mouse state and coordinates
    //// Check if the left mouse button is pressed
    //if (mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT))
    //{
    //    // Convert mouse position to grid coordinates
    //    size_t gridX = static_cast<size_t>(mouseX);
    //    size_t gridY = static_cast<size_t>(mouseY);
    //    // Radius for the circular brush
    //    int radius = m_BrushSize / 2;
    //    // Iterate over a square bounding the circle, but only place particles inside the circle
    //    for (int dx = -radius; dx <= radius; ++dx)
    //    {
    //        for (int dy = -radius; dy <= radius; ++dy)
    //        {
    //            // Calculate the distance from the center (gridX, gridY)
    //            int distanceSquared = dx * dx + dy * dy;
    //            // If the point is inside the radius (circle equation: x² + y² <= r²)
    //            if (distanceSquared <= radius * radius)
    //            {
    //                size_t brushX = gridX + dx;
    //                size_t brushY = gridY + dy;
    //                // Place a particle if within grid bounds
    //                if (brushX < m_pWindow->GetColumns() && brushY < m_pWindow->GetRows())
    //                {
    //                    //ServiceLocator::GetSandSimulator().PlaceParticle(brushX, brushY, std::make_unique<E_Sand>());
    //                }
    //            }
    //        }
    //    }
    //}
}

void Game::Update(float deltaTime)
{
    // Game logic update can be implemented here
    ServiceLocator::GetSandSimulator().Update(deltaTime);
}

void Game::Render() const
{
    m_pWindow->Clear();  // Clear window to a specific color

    ServiceLocator::GetSandSimulator().Render();

    m_pWindow->Update(); // Render the updated content
}
