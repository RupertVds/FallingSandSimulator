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
    //m_pWindow = new Window("RVDS - Falling Sand Simulator", 1920, 1080);
    if (!m_pWindow->Init())
    {
        std::cerr << "Failed to initialize window." << std::endl;
        m_IsRunning = false;
    }

    //ServiceLocator::RegisterSandSimulation(std::make_unique<CPUSandSimulation>(GridInfo{ glm::ivec2{10, 10}, 320, 480, 2 }, m_pWindow));
    int cellSize{ 4 };
    ServiceLocator::RegisterSandSimulation(std::make_unique<CPUSandSimulation>(GridInfo{ glm::ivec2{0, 0}, m_pWindow->GetHeight() / cellSize, m_pWindow->GetWidth() / cellSize, cellSize }, m_pWindow));
    //ServiceLocator::RegisterSandSimulation(std::make_unique<CPUSandSimulation>(GridInfo{ glm::ivec2{10, 10}, 500, 500, 2 }, m_pWindow));
    //ServiceLocator::RegisterSandSimulation(std::make_unique<CPUSandSimulation>(GridInfo{ glm::ivec2{10, 10}, 40, 70, 16 }, m_pWindow));
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
    ServiceLocator::GetSandSimulator().SetFixedTimeStep(SIMULATION_TIME_STEP);

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
            FixedUpdate();   // Update simulation with fixed time step
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
