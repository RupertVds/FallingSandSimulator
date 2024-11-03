#include "Behavior.h"
#include "Element.h"
#include "Grid.h"

MovableSolid::MovableSolid(float mass, float inertia)
    : mass(mass), inertia(inertia)
{
}

void MovableSolid::Update(Element& element, Grid& grid, int x, int y)
{
    // Example: Move down if the cell below is empty
    if (grid.IsWithinBounds(x + 1, y) && grid.GetCell(x + 1, y).IsEmpty())
    {
        grid.MoveElement(x, y, x + 1, y);
    }
}

Behavior::~Behavior()
{
}
