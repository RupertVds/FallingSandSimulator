#include "CPUSandSimulation.h"
#include <iostream>
#include <SDL_surface.h>
#include <SDL_gesture.h>
#include <random>

CPUSandSimulation::CPUSandSimulation(size_t width, size_t height)
	:
	m_Width{width},
	m_Height{height},
	m_Grid{ m_Width, std::vector<int>(height, 0)}
{
	// Check if width and height are valid
	if (width <= 0 || height <= 0)
	{
		throw std::runtime_error("Width and height must be greater than 0");
	}

	// Make sure we can safely access the middle row
	if (width / 2 >= m_Grid.size() || m_Grid[width / 2].size() == 0)
	{
		throw std::runtime_error("Grid dimensions are out of range");
	}

	// Initialize some sand particles for testing
	m_Grid[width / 2][0] = 1;  // Drop a particle at the top center
}

void CPUSandSimulation::Update()
{
    std::mt19937 gen(m_rd());
    std::uniform_int_distribution<> dist(0, 1);  // Generates 0 or 1 randomly

    static bool leftToRight = true;  // Toggle direction

    // Loop over rows from bottom to top
    for (int y = static_cast<int>(m_Height) - 2; y >= 0; --y)
    {
        if (leftToRight)
        {
            // Process grid left to right
            for (int x = 0; x < static_cast<int>(m_Width); ++x)
            {
                ProcessSandParticle(x, y, gen, dist);
            }
        }
        else
        {
            // Process grid right to left
            for (int x = static_cast<int>(m_Width) - 1; x >= 0; --x)
            {
                ProcessSandParticle(x, y, gen, dist);
            }
        }
    }

    // Toggle the direction for the next frame
    leftToRight = !leftToRight;
}

    
void CPUSandSimulation::Render(SDL_Surface* surface) const
{
	// Lock the surface before directly manipulating pixels
	if (SDL_MUSTLOCK(surface))
	{
		SDL_LockSurface(surface);
	}

	Uint32* pixels = static_cast<Uint32*>(surface->pixels);
	int pitch = surface->pitch / 4;  // pitch is in bytes, so divide by 4 to get pixel width

	// Iterate through the grid and draw pixels based on particle presence
	for (size_t y = 0; y < m_Height; ++y)
	{
		for (size_t x = 0; x < m_Width; ++x)
		{
			Uint32 color = (m_Grid[x][y] == 1) ? SDL_MapRGB(surface->format, 227, 186, 102)  // Yellow for sand particles
				: SDL_MapRGB(surface->format, 161, 163, 181);    // Black for empty space

			// Set pixel color at position (x, y) in the SDL surface
			pixels[y * pitch + x] = color;
		}
	}

	// Unlock the surface after manipulation
	if (SDL_MUSTLOCK(surface))
	{
		SDL_UnlockSurface(surface);
	}
}

void CPUSandSimulation::PlaceParticle(size_t x, size_t y)
{
	if (x < m_Width && y < m_Height)
	{
		m_Grid[x][y] = 1;
	}
}

// Separate method to process each sand particle at (x, y)
void CPUSandSimulation::ProcessSandParticle(int x, int y, std::mt19937& gen, std::uniform_int_distribution<>& dist)
{
    // Only process if the current cell contains a sand particle
    if (m_Grid[x][y] == 1)
    {
        // Check if there's space below the particle (fall straight down)
        if (y + 1 < m_Height && m_Grid[x][y + 1] == 0)
        {
            m_Grid[x][y + 1] = 1;
            m_Grid[x][y] = 0;
        }
        // If space below is blocked, check diagonal movement
        else if (y + 1 < m_Height && m_Grid[x][y + 1] == 1)
        {
            bool canFallLeft = (x - 1 >= 0 && m_Grid[x - 1][y + 1] == 0);  // Check left down
            bool canFallRight = (x + 1 < m_Width && m_Grid[x + 1][y + 1] == 0);  // Check right down

            // Randomly choose between left and right if both are available
            if (canFallLeft && canFallRight)
            {
                if (dist(gen) == 0)
                {
                    m_Grid[x - 1][y + 1] = 1; // Move left
                }
                else
                {
                    m_Grid[x + 1][y + 1] = 1; // Move right
                }
                m_Grid[x][y] = 0; // Clear current position
            }
            else if (canFallLeft)
            {
                // Only move left if right is blocked
                m_Grid[x - 1][y + 1] = 1;
                m_Grid[x][y] = 0;
            }
            else if (canFallRight)
            {
                // Only move right if left is blocked
                m_Grid[x + 1][y + 1] = 1;
                m_Grid[x][y] = 0;
            }
        }
    }
}
