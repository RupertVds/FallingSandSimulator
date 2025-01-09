#include "CPUSandSimulation.h"
#include <iostream>
#include <SDL_surface.h>
#include <SDL_gesture.h>
#include <random>
#include <chrono>
#include <thread>
#include "InputManager.h"
#include "ServiceLocator.h"

CPUSandSimulation::CPUSandSimulation(const GridInfo& gridInfo, Window* window)
    :
    m_pGrid{ std::make_unique<Grid>(gridInfo) },
    m_pWindow{ window }
{
	// Check if width and height are valid
	if (gridInfo.rows <= 0 || gridInfo.columns <= 0)
	{
		throw std::runtime_error("Rows and Columns must be greater than 0");
	}

    Init();
}

void CPUSandSimulation::Init()
{
    m_pGrid->ClearGrid();
    m_pGrid->Init();
}

void CPUSandSimulation::Update()
{
    if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_SPACE))
    {
        m_IsSimulating = !m_IsSimulating;
        std::cout << "TOGGLED SIMULATING\n";
    }

    if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_S))
    {
        m_pGrid->FixedUpdate();
        std::cout << "STEPPED SIMULATION\n";
    }

    m_pGrid->Update();
}

void CPUSandSimulation::FixedUpdate()
{
    if (m_IsSimulating)
    {
        m_pGrid->FixedUpdate();
    }
}

void CPUSandSimulation::Render() const
{
    m_pGrid->Render(m_pWindow);
}

bool CPUSandSimulation::IsActive() const
{
    return m_IsSimulating;
}

float CPUSandSimulation::GetFixedTimeStep() const
{
    return m_FixedTimeStep;
}

void CPUSandSimulation::SetFixedTimeStep(float fixedTimeStep)
{
    m_FixedTimeStep = fixedTimeStep;
}
