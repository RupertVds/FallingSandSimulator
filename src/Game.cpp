#include "Game.h"
#include <iostream>
#include "ServiceLocator.h"
#include "CPUSandSimulation.h"
#include "InputManager.h"
#include <thread>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer.h"

Game::Game()
    : m_pWindow(nullptr), m_IsRunning(true)
{
    m_pWindow = new Window("RVDS - Falling Sand Simulator", 1700, 720);
    //m_pWindow = new Window("RVDS - Falling Sand Simulator", 1920, 1080);
    if (!m_pWindow->Init())
    {
        std::cerr << "Failed to initialize window." << std::endl;
        m_IsRunning = false;
    }

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // Configure ImGui style
    ImGui::StyleColorsDark();

    // Initialize ImGui SDL2 and SDL_Renderer bindings
    ImGui_ImplSDL2_InitForSDLRenderer(m_pWindow->GetSDLWindow(), m_pWindow->GetSDLRenderer());
    ImGui_ImplSDLRenderer_Init(m_pWindow->GetSDLRenderer());

    //ServiceLocator::RegisterSandSimulation(std::make_unique<CPUSandSimulation>(GridInfo{ glm::ivec2{10, 10}, 320, 480, 2 }, m_pWindow));
    int cellSize{ 2 };
    //ServiceLocator::RegisterSandSimulation(std::make_unique<CPUSandSimulation>(GridInfo{ glm::ivec2{0, 0}, m_pWindow->GetHeight() / cellSize, m_pWindow->GetWidth() / cellSize, cellSize }, m_pWindow));
    ServiceLocator::RegisterSandSimulation(std::make_unique<CPUSandSimulation>(GridInfo{ glm::ivec2{20, 20}, 600 / cellSize, 1000 / cellSize, cellSize }, m_pWindow));
    //ServiceLocator::RegisterSandSimulation(std::make_unique<CPUSandSimulation>(GridInfo{ glm::ivec2{10, 10}, 500, 500, 2 }, m_pWindow));
    //ServiceLocator::RegisterSandSimulation(std::make_unique<CPUSandSimulation>(GridInfo{ glm::ivec2{10, 10}, 40, 70, 16 }, m_pWindow));
}

Game::~Game()
{
    // Shutdown ImGui backends
    ImGui_ImplSDLRenderer_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

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
    constexpr float FPS_UPDATE_INTERVAL = 0.5f;

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
            int averageFPS = frameCount / fpsAccumulator;
            //std::cout << "Average FPS: " << averageFPS << std::endl;
            std::string title = "FPS: " + std::to_string(averageFPS);
            SDL_SetWindowTitle(m_pWindow->GetSDLWindow(), title.c_str());

            // Reset accumulator and frame count
            fpsAccumulator = 0.0f;
            frameCount = 0;
        }

        ProcessInput();

        while (lag >= SIMULATION_TIME_STEP)
        {
            FixedUpdate();  
            lag -= SIMULATION_TIME_STEP;
        }

        Update();
        Render();

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
        // Pass events to ImGui
        ImGui_ImplSDL2_ProcessEvent(&e);

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

    // Start a new ImGui frame
    ImGui_ImplSDLRenderer_NewFrame();
    ImGui_ImplSDL2_NewFrame(m_pWindow->GetSDLWindow());
    ImGui::NewFrame();

    ServiceLocator::GetSandSimulator().Render();

    //ImGui::ShowDemoWindow(); // Show demo window! :)


    // End the ImGui frame and render
    ImGui::Render();
    ImGui_ImplSDLRenderer_RenderDrawData(ImGui::GetDrawData());

    m_pWindow->Update(); // Render the updated content
}