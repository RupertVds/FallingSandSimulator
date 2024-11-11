#include "Game.h"
#include <iostream>
#include "ServiceLocator.h"
#include "CPUSandSimulation.h"
#include "InputManager.h"
#include <thread>

Game::Game()
    : m_pWindow(nullptr), m_IsRunning(true)
{
    m_pWindow = new Window("RVDS - Falling Sand Simulator", 1280, 720);
    if (!m_pWindow->Init())
    {
        std::cerr << "Failed to initialize window." << std::endl;
        m_IsRunning = false;
    }

    // Register the CPU-based sand simulation as the default
    // grid with: pos: x=0;y=20 , rows=11, cols=20, cellsize=10
    //ServiceLocator::RegisterSandSimulation(std::make_unique<CPUSandSimulation>(GridInfo{ glm::ivec2{10, 10}, 320, 480, 2 }, m_pWindow));
    ServiceLocator::RegisterSandSimulation(std::make_unique<CPUSandSimulation>(GridInfo{ glm::ivec2{10, 10}, 80, 120, 8 }, m_pWindow));
}

Game::~Game()
{
    delete m_pWindow;
}

void Game::Run()
{
    constexpr bool USE_VSYNC{ true };
    constexpr bool CAP_FPS{ true };
    constexpr float TARGETFPS{ 144.0f };
    constexpr double TARGET_FRAME_DURATION = 1.0 / TARGETFPS;
    constexpr float SIMULATION_TIME_STEP = 1.0f / 60.0f;

    float lag = 0.0f;
    float fpsAccumulator = 0.0f;
    int frameCount = 0;
    constexpr float FPS_UPDATE_INTERVAL = 1.0f;  // Update and print FPS every second

    SDL_GL_SetSwapInterval(USE_VSYNC ? 1 : 0);
    

    auto lastFrameTime = std::chrono::high_resolution_clock::now();

    while (m_IsRunning)
    {
        // Calculate delta time
        auto currentFrameTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentFrameTime - lastFrameTime).count();
        lastFrameTime = currentFrameTime;
        lag += deltaTime;

        // FPS calculation
        fpsAccumulator += deltaTime;
        frameCount++;

        // Print FPS every second
        if (fpsAccumulator >= FPS_UPDATE_INTERVAL)
        {
            float averageFPS = frameCount / fpsAccumulator;
            std::cout << "Average FPS: " << averageFPS << std::endl;

            // Reset accumulator and frame count
            fpsAccumulator = 0.0f;
            frameCount = 0;
        }

        ProcessInput();                // Process user inputs and events

        while (lag >= SIMULATION_TIME_STEP)
        {
            FixedUpdate();             // Update simulation with fixed time step
            lag -= SIMULATION_TIME_STEP;
        }

        Update();                      // General updates based on delta time
        Render();                      // Render everything

        // FPS Capping Logic
        if (CAP_FPS && !USE_VSYNC)
        {
            auto frameEndTime = std::chrono::high_resolution_clock::now();
            double frameDuration = std::chrono::duration<double>(frameEndTime - currentFrameTime).count();
            double sleepDuration = TARGET_FRAME_DURATION - frameDuration;

            if (sleepDuration > 0.0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleepDuration * 1000)));
            }
        }
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

        InputManager::GetInstance().HandleSDLEvent(e);
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

void Game::Update()
{
    // Game logic update can be implemented here
    ServiceLocator::GetSandSimulator().Update();
    InputManager::GetInstance().ResetScrollState();

}

void Game::FixedUpdate()
{
    ServiceLocator::GetSandSimulator().FixedUpdate();

}

void Game::Render() const
{
    m_pWindow->Clear();  // Clear window to a specific color

    ServiceLocator::GetSandSimulator().Render();

    m_pWindow->Update(); // Render the updated content
}
