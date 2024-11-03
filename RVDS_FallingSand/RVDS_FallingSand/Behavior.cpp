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
    if (grid.IsWithinBounds(x, y + 1) && grid.GetCell(x, y + 1).IsEmpty())
    {
        grid.MoveElement(x, y, x, y + 1);
    }


}

Behavior::~Behavior()
{
}
