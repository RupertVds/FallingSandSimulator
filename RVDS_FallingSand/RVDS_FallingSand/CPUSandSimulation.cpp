#include "CPUSandSimulation.h"
#include <iostream>
#include <SDL_surface.h>
#include <SDL_gesture.h>
#include <random>
#include <chrono>
#include <thread>

CPUSandSimulation::CPUSandSimulation(const GridInfo& gridInfo, Window* window)
	:
    m_pGrid{std::make_unique<Grid>( gridInfo )},
    m_pWindow{window}
{
	// Check if width and height are valid
	if (gridInfo.rows <= 0 || gridInfo.columns <= 0)
	{
		throw std::runtime_error("Rows and Columns must be greater than 0");
	}

	// Initialize some sand particles for testing
    auto sand = std::make_shared<Element>("Sand", glm::vec3{0,0,0});
    sand->AddBehavior<MovableSolid>(5.0f, 0.2f);

    m_pGrid->GetCell(10, 3).m_pElement = sand;
    m_pGrid->GetCell(11, 3).m_pElement = sand;
    m_pGrid->GetCell(12, 3).m_pElement = sand;
    m_pGrid->GetCell(13, 3).m_pElement = sand;
    m_pGrid->GetCell(14, 3).m_pElement = sand;
    m_pGrid->GetCell(15, 3).m_pElement = sand;
    m_pGrid->GetCell(16, 3).m_pElement = sand;
    m_pGrid->GetCell(26, 3).m_pElement = sand;
}

void CPUSandSimulation::Update(float deltaTime)
{
    m_pGrid->Update();
    std::this_thread::sleep_for(std::chrono::seconds(static_cast<long long>(1)));

}

void CPUSandSimulation::Render() const
{
    m_pGrid->Render(m_pWindow);
}

void CPUSandSimulation::PlaceParticle(size_t x, size_t y, std::unique_ptr<Element>&& element)
{

}

void CPUSandSimulation::ProcessSandParticle(int x, int y, std::mt19937& gen, std::uniform_int_distribution<>& dist)
{
    //// Only process if the current cell contains a sand particle
    //if (m_Grid[x][y] == 1)
    //{
    //    // Check if there's space directly below the particle (fall straight down)
    //    if (y + 1 < m_Height && m_Grid[x][y + 1] == 0)
    //    {
    //        m_Grid[x][y + 1] = 1;
    //        m_Grid[x][y] = 0;
    //    }
    //    // If space below is blocked, check diagonal movement
    //    else if (y + 1 < m_Height && m_Grid[x][y + 1] == 1)
    //    {
    //        bool canFallLeft = (x - 1 >= 0 && m_Grid[x - 1][y + 1] == 0);  // Check left down
    //        bool canFallRight = (x + 1 < m_Width && m_Grid[x + 1][y + 1] == 0);  // Check right down
    //        // Only allow diagonal movement if the particle is on solid ground
    //        if (canFallLeft && canFallRight)
    //        {
    //            if (dist(gen) == 0)
    //            {
    //                m_Grid[x - 1][y + 1] = 1; // Move left
    //            }
    //            else
    //            {
    //                m_Grid[x + 1][y + 1] = 1; // Move right
    //            }
    //            m_Grid[x][y] = 0; // Clear current position
    //        }
    //        else if (canFallLeft)
    //        {
    //            // Only move left if right is blocked
    //            m_Grid[x - 1][y + 1] = 1;
    //            m_Grid[x][y] = 0;
    //        }
    //        else if (canFallRight)
    //        {
    //            // Only move right if left is blocked
    //            m_Grid[x + 1][y + 1] = 1;
    //            m_Grid[x][y] = 0;
    //        }
    //    }
    //    // Prevent diagonal falling when there's space directly below
    //    else if (y + 1 < m_Height && m_Grid[x][y + 1] == 0)
    //    {
    //        // Fall straight down
    //        m_Grid[x][y + 1] = 1;
    //        m_Grid[x][y] = 0;
    //    }
    //}
}





//std::mt19937 gen(m_Rd());
//std::uniform_int_distribution<> dist(0, 1);  // Randomizer for left/right
//// Loop through particles
//for (int y = static_cast<int>(m_Height) - 2; y >= 0; --y)
//{
//    for (int x = 0; x < static_cast<int>(m_Width); ++x)
//    {
//        if (m_Grid[x][y] == 1)
//        {
//            // Accumulate time for this particle
//            m_TimeAccumulator[x][y] += deltaTime * m_FallSpeed;
//
//            // Check if the particle has accumulated enough time to move
//            if (m_TimeAccumulator[x][y] >= 1.0f)
//            {
//                int fallSteps = static_cast<int>(m_TimeAccumulator[x][y]);
//                m_TimeAccumulator[x][y] -= fallSteps;
//
//                // Move particle based on fall steps (the number of pixels to fall)
//                for (int i = 0; i < fallSteps; ++i)
//                {
//                    if (y + 1 < m_Height && m_Grid[x][y + 1] == 0)
//                    {
//                        m_Grid[x][y + 1] = 1;
//                        m_Grid[x][y] = 0;
//                    }
//                    // Handle diagonal fall, as before
//                    else if (y + 1 < m_Height && m_Grid[x][y + 1] == 1)
//                    {
//                        bool canFallLeft = (x - 1 >= 0 && m_Grid[x - 1][y + 1] == 0);
//                        bool canFallRight = (x + 1 < m_Width && m_Grid[x + 1][y + 1] == 0);
//
//                        if (canFallLeft && canFallRight)
//                        {
//                            if (dist(gen) == 0)
//                            {
//                                m_Grid[x - 1][y + 1] = 1;
//                            }
//                            else
//                            {
//                                m_Grid[x + 1][y + 1] = 1;
//                            }
//                            m_Grid[x][y] = 0;
//                        }
//                        else if (canFallLeft)
//                        {
//                            m_Grid[x - 1][y + 1] = 1;
//                            m_Grid[x][y] = 0;
//                        }
//                        else if (canFallRight)
//                        {
//                            m_Grid[x + 1][y + 1] = 1;
//                            m_Grid[x][y] = 0;
//                        }
//                    }
//                }
//            }
//        }
//    }
//}