#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "Utils.h"
#include <algorithm>

#define SOUTH glm::ivec2{1, 0}
#define SOUTH_WEST glm::ivec2{1, -1}
#define SOUTH_EAST glm::ivec2{1, 1}
#define NORTH glm::ivec2{-1, 0}
#define NORTH_WEST glm::ivec2{-1, -1}
#define NORTH_EAST glm::ivec2{-1, 1}
#define WEST glm::ivec2{0, -1}
#define EAST glm::ivec2{0, 1}

// these are all systems that are applied on the components of the elements
template <typename ComponentType>
bool HasComponent(const Element* element, const std::string& componentName)
{
    return element && element->definition->components.count(componentName);
}

template <typename ComponentType>
const ComponentType* GetComponent(const Element* element, const std::string& componentName)
{
    auto it = element->definition->components.find(componentName);
    if (it != element->definition->components.end())
    {
        return std::get_if<ComponentType>(&it->second); // Safely retrieve the const component
    }
    return nullptr; // Return nullptr if the component doesn't exist
}

void ProcessSolid(Element* element, int x, int y, Grid& grid);

void ProcessLiquid(Element* element, int x, int y, Grid& grid, float dispersionRate);

void ProcessGas(Element* element, int x, int y, Grid& grid);

bool CanSolidReachTarget(glm::ivec2 start, glm::ivec2 target, Grid& grid);

void UpdateGridElements(Grid& grid)
{
    constexpr float GRAVITY{ 9.8f };
    // Traverse the grid in one pass
    for (int x = grid.GetRows() - 1; x >= 0; --x)
    {
        // Determine iteration direction based on the frame count
        int startY = grid.IsEvenFrame() ? 0 : grid.GetColumns() - 1;
        int endY = grid.IsEvenFrame() ? grid.GetColumns() : -1;
        int step = grid.IsEvenFrame() ? 1 : -1;

        for (int y = startY; y != endY; y += step)
        {
            if (grid.IsEmpty(x, y)) continue;

            Element* element = grid.GetElementData(x, y);
            if (element->hasMoved) continue;


            // HANDLE ALL VELOCITY BASED COMPONENTS
            // Process Gravity
            if (HasComponent<GravityComp>(element, "Gravity"))
            {
                auto* comp = GetComponent<GravityComp>(element, "Gravity");
                element->velocity.x += GRAVITY * comp->gravityScale * ServiceLocator::GetSandSimulator().GetFixedTimeStep();
            }

            // HANDLE MODIFIER COMPONENTS
            // Additional components (flammable, etc.)
            if (HasComponent<FlammableComp>(element, "Flammable"))
            {
                //handle flammable logic
            }

            // Calculate the target position based on current velocity
            glm::ivec2 startPos = { x, y };
            glm::ivec2 targetPos = {
                x + static_cast<int>(element->velocity.x), // Vertical movement (rows)
                y + static_cast<int>(element->velocity.y)  // Vertical movement (columns)
            };

            // Clamp target position to grid bounds
            targetPos.x = std::clamp(targetPos.x, 0, grid.GetRows() - 1);
            targetPos.y = std::clamp(targetPos.y, 0, grid.GetColumns() - 1);


            // Cache if the element is Solid, Liquid or Gas to reduce checking it in the Bresenham's algorithm
            bool isSolid = HasComponent<SolidComp>(element, "Solid");
            bool isLiquid = false;
            bool isGas = false;
            
            if (!isSolid)
            {
                isLiquid = HasComponent<LiquidComp>(element, "Liquid");
            }
            else if (!isLiquid)
            {
                isGas = HasComponent<GasComp>(element, "Gas");
            }

            // Use Bresenham's line to check for available positions from start to target
            bool blocked = false;
			glm::ivec2 lastValidPos{ startPos };

            // only do bresenham if velocity is greater or equal  to/than 2
            if (element->velocity.x > 2 || element->velocity.y > 2)
            {
				BresenhamLine(startPos, targetPos, [&](int currentX, int currentY)
					{
						glm::ivec2 currentPos{ currentX, currentY };
						glm::ivec2 direction = currentPos - lastValidPos;

						if (!grid.IsWithinBounds(currentX, currentY))
						{
							blocked = true;
							return false; // Stop traversal if out of bounds
						}

						// Evaluate if this step can be reached from the last valid position
						if (CanSolidReachTarget(lastValidPos, currentPos, grid))
						{
							// If reachable, update lastValidPos to current position
							lastValidPos = currentPos;
							return true; // Continue traversal
						}

						// If the step is not reachable, mark blocked and stop traversal
						blocked = true;
						return false;
					});

				if (blocked)
				{
					element->velocity = {};
				}

				// now place element at last valid pos
				grid.SwapElements(x, y, lastValidPos.x, lastValidPos.y);
            }

            // now velocity has placed element at correct position
            // now we process the interaction with other elements
            // NOW DO OUR FINAL MAIN COMPONENTS
			if (isSolid)
			{
				ProcessSolid(element, lastValidPos.x, lastValidPos.y, grid);
			}
			else if (isLiquid)
			{
				auto* liquidComp = GetComponent<LiquidComp>(element, "Liquid");
				ProcessLiquid(element, lastValidPos.x, lastValidPos.y, grid, liquidComp->dispersionRate);
			}
			else if (isGas)
			{
				ProcessGas(element, lastValidPos.x, lastValidPos.y, grid);
			}
        }
    }

    // Reset the "hasMoved" flag for all elements
    for (int x = 0; x < grid.GetRows(); ++x)
    {
        for (int y = 0; y < grid.GetColumns(); ++y)
        {
            Element* element = grid.GetElementData(x, y);
            if (element)
            {
                element->hasMoved = false;
            }
        }
    }
}

bool CanSolidReachTarget(glm::ivec2 start, glm::ivec2 target, Grid& grid)
{
	glm::ivec2 direction = target - start;

	if (!grid.IsWithinBounds(target)) 
    {
		return false; // Target is outside the grid bounds
	}

	// If the direction is valid, determine if the target position is reachable
	if (direction == SOUTH) {
		return grid.IsEmpty(target); // Can move south if the target cell is empty
	}
	else if (direction == NORTH) {
		return grid.IsEmpty(target);
	}
	else if (direction == EAST) {
		return grid.IsEmpty(target);
	}
	else if (direction == WEST) {
		return grid.IsEmpty(target);
	}
	else if (direction == NORTH_EAST) {
		return grid.IsEmpty(target);
	}
	else if (direction == NORTH_WEST) {
		return grid.IsEmpty(target);
	}
	else if (direction == SOUTH_EAST) {
		return grid.IsEmpty(target);
	}
	else if (direction == SOUTH_WEST) {
		return grid.IsEmpty(target);
	}

	// If the direction is zero (start == target), disallow it
	if (direction == glm::ivec2{ 0, 0 }) {
		//std::cout << "Target is current position (unexpected).\n";
		return true;
	}

	return false; // Default to unreachable if no valid direction is matched
}


void ProcessGas(Element* element, int x, int y, Grid& grid)
{
    // Process Gas behavior
    if (HasComponent<GasComp>(element, "Gas"))
    {
        const auto* comp = GetComponent<GasComp>(element, "Gas");
        int upX = x - 1;

        if (grid.IsWithinBounds(upX, y) && grid.IsEmpty(upX, y)) {
            grid.SwapElements(x, y, upX, y);
            element->hasMoved = true;
        }
        else if (grid.IsWithinBounds(upX, y - 1) && grid.IsEmpty(upX, y - 1)) { // Up-left
            grid.SwapElements(x, y, upX, y - 1);
            element->hasMoved = true;
        }
        else if (grid.IsWithinBounds(upX, y + 1) && grid.IsEmpty(upX, y + 1)) { // Up-right
            grid.SwapElements(x, y, upX, y + 1);
            element->hasMoved = true;
        }
    }
}

void ProcessLiquid(Element* element, int x, int y, Grid& grid, float dispersionRate)
{
    // Check if the element below (downwards) is empty
    if (x < grid.GetRows() - 1 && grid.IsEmpty(x + 1, y))
    {
        grid.SwapElements(x, y, x + 1, y); // Move down
        return; // Movement was successful
    }
    else
    {
        // Randomize horizontal preference to simulate natural spread
        bool moveRightFirst = (rand() % 2 == 0);

        // Helper function to check diagonal movement
        auto tryMoveDiagonal = [&](int dx, int dy) -> bool 
            {
            int targetX = x + 1; // Always check one row down
            int targetY = y + dy;
            if (grid.IsWithinBounds(targetX, targetY) && grid.IsEmpty(targetX, targetY))
            {
                // Check if the horizontal neighbor blocks diagonal movement
                int neighborX = x;
                int neighborY = y + dy;
                if (grid.IsWithinBounds(neighborX, neighborY) && grid.IsEmpty(neighborX, neighborY))
                {
                    grid.SwapElements(x, y, targetX, targetY); // Move diagonally
                    return true; // Movement was successful
                }
            }
            return false;
            };

        // Attempt diagonal movement
        if (moveRightFirst)
        {
            if (tryMoveDiagonal(1, 1)) return; // Try down-right
            if (tryMoveDiagonal(1, -1)) return; // Try down-left
        }
        else
        {
            if (tryMoveDiagonal(1, -1)) return; // Try down-left
            if (tryMoveDiagonal(1, 1)) return; // Try down-right
        }

        // Attempt horizontal dispersion
        auto tryHorizontal = [&](int dy) -> bool
            {
            int newY = y + dy;
            if (grid.IsWithinBounds(x, newY) && grid.IsEmpty(x, newY))
            {
                grid.SwapElements(x, y, x, newY);
                return true; // Movement was successful
            }
            return false; // Movement was blocked
            };

        int maxDispersion = static_cast<int>(dispersionRate);
        for (int step = 1; step <= maxDispersion; ++step)
        {
            if (moveRightFirst)
            {
                if (tryHorizontal(step)) return; // Spread right
                if (tryHorizontal(-step)) return; // Spread left
            }
            else
            {
                if (tryHorizontal(-step)) return; // Spread left
                if (tryHorizontal(step)) return; // Spread right
            }
        }
    }
}

void ProcessSolid(Element* element, int x, int y, Grid& grid)
{
    // Check if the element below (downwards) is empty or has a liquid component
    if (x < grid.GetRows() - 1 && (grid.IsEmpty(x + 1, y) || HasComponent<LiquidComp>(grid.GetElementData(x + 1, y), "Liquid")))
    {
        grid.SwapElements(x, y, x + 1, y); // Move down
        return; // Movement was successful
    }
    else
    {
        // Randomize horizontal preference to simulate natural spread
        bool moveRightFirst = (rand() % 2 == 0);

        // Helper function to check diagonal movement
        auto tryMoveDiagonal = [&](int dx, int dy) -> bool 
            {
            int targetX = x + 1; // Always check one row down
            int targetY = y + dy;
            if (grid.IsWithinBounds(targetX, targetY) && (grid.IsEmpty(targetX, targetY) || HasComponent<LiquidComp>(grid.GetElementData(targetX, targetY), "Liquid")))
            {
                // Check if the horizontal neighbor blocks diagonal movement
                int neighborX = x;
                int neighborY = y + dy;
                if (grid.IsWithinBounds(neighborX, neighborY) && (grid.IsEmpty(neighborX, neighborY) || HasComponent<LiquidComp>(grid.GetElementData(neighborX, neighborY), "Liquid")))
                {
                    grid.SwapElements(x, y, targetX, targetY); // Move diagonally
                    return true; // Movement was successful
                }
            }
            return false; // Movement was blocked
            };

        // Attempt diagonal movement
        if (moveRightFirst)
        {
            if (tryMoveDiagonal(1, 1)) return; // Try down-right
            if (tryMoveDiagonal(1, -1)) return; // Try down-left
        }
        else
        {
            if (tryMoveDiagonal(1, -1)) return; // Try down-left
            if (tryMoveDiagonal(1, 1)) return; // Try down-right
        }
    }
}


#endif // !SYSTEMS_H