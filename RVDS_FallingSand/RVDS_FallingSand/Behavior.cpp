#include "Behavior.h"
#include "Element.h"
#include "Grid.h"

MovableSolid::MovableSolid(float mass, float inertia)
    : mass(mass), inertia(inertia)
{
}

void MovableSolid::Update(Element& element, Grid& grid, int x, int y)
{
    // Check if the cell below is within bounds, empty in current grid, and empty in next grid
    if (grid.IsWithinBounds(x - 1, y) && grid.GetCurrentGrid()[x - 1][y]->IsEmpty() && grid.GetNextGrid()[x - 1][y]->IsEmpty())
    {
        // Move the element down in the next grid by transferring ownership
        grid.GetNextGrid()[x - 1][y]->m_pElement = std::move(grid.GetCurrentGrid()[x][y]->m_pElement);
    }
    else if (grid.GetNextGrid()[x][y]->IsEmpty())
    {
        // If the element doesn’t move, transfer ownership to the same cell in nextGrid
        grid.GetNextGrid()[x][y]->m_pElement = std::move(grid.GetCurrentGrid()[x][y]->m_pElement);
    }

    //// Check if the cell below is within bounds, empty in current grid, and empty in next grid
    //if (grid.IsWithinBounds(x + 1, y) && grid.GetCurrentGrid()[x + 1][y]->IsEmpty() && grid.GetNextGrid()[x + 1][y]->IsEmpty())
    //{
    //    // Move the element down in the next grid by transferring ownership
    //    grid.GetNextGrid()[x + 1][y]->m_pElement = std::move(grid.GetCurrentGrid()[x][y]->m_pElement);
    //}
    //else if (grid.GetNextGrid()[x][y]->IsEmpty())
    //{
    //    // If the element doesn’t move, transfer ownership to the same cell in nextGrid
    //    grid.GetNextGrid()[x][y]->m_pElement = std::move(grid.GetCurrentGrid()[x][y]->m_pElement);
    //}
}

Behavior::~Behavior()
{
}
