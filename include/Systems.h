#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "Utils.h"
#include <algorithm>

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

void ProcessSolid(Element* element, int x, int y, Grid& grid, bool& blocked);

void ProcessLiquid(Element* element, int x, int y, Grid& grid, bool& blocked, float dispersionRate);

void ProcessGas(Element* element, int x, int y, Grid& grid);

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


            // Cache if the element is Solid, Liquid or Gas to reduce checking it in the bresenhams algorithm
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
            BresenhamLine(startPos, targetPos, [&](int currentX, int currentY)
                {
                    // Cache the result of grid.IsEmpty() to avoid multiple calls to grid.IsEmpty()
                    bool isEmpty = grid.IsEmpty(currentX, currentY);
                    Element* currentElement = isEmpty ? nullptr : grid.GetElementData(currentX, currentY);

                    // If the cell is not empty, perform the movement checks
                    if (currentElement && !isEmpty)
                    {
                        // NOW DO OUR FINAL MAIN COMPONENTS
                        if (isSolid)
                        {
                            ProcessSolid(element, currentX, currentY, grid, blocked);
                        }
                        else if (isLiquid)
                        {
                            auto* liquidComp = GetComponent<LiquidComp>(element, "Liquid");
                            ProcessLiquid(element, currentX, currentY, grid, blocked, liquidComp->dispersionRate);
                        }
                        else if (isGas)
                        {
                            ProcessGas(element, currentX, currentY, grid);
                        }
                    }

                    if (blocked)
                    {
                        element->velocity = {};
                        return false;
                    }
                    return true;
                });


        }
    }

    // Reset the "hasMoved" flag for all elements
    for (int x = 0; x < grid.GetRows(); ++x)
    {
        for (int y = 0; y < grid.GetColumns(); ++y)
        {
            Element* element = grid.GetElementData(x, y);
            if (element) {
                element->hasMoved = false;
            }
        }
    }
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

void ProcessLiquid(Element* element, int x, int y, Grid& grid, bool& blocked, float dispersionRate)
{
    // Step 1: Try moving down directly
    if (x < grid.GetRows() - 1 && grid.IsEmpty(x + 1, y))
    {
        grid.MoveElement(x, y, x + 1, y);
        return; // Successful downward movement
    }

    // Step 2: Try diagonal downward movement
    bool moveRightFirst = (rand() % 2 == 0);

    auto tryDiagonal = [&](int dx, int dy) -> bool
        {
            int newX = x + dx, newY = y + dy;
            if (grid.IsWithinBounds(newX, newY) && grid.IsEmpty(newX, newY))
            {
                grid.MoveElement(x, y, newX, newY);
                return true; // Successful diagonal movement
            }
            return false;
        };

    if (moveRightFirst)
    {
        if (tryDiagonal(1, 1)) return; // Down-right
        if (tryDiagonal(1, -1)) return; // Down-left
    }
    else
    {
        if (tryDiagonal(1, -1)) return; // Down-left
        if (tryDiagonal(1, 1)) return; // Down-right
    }

    // Step 3: If downward movement fails, attempt horizontal dispersion
    auto tryHorizontal = [&](int dy) -> bool
        {
            int newY = y + dy;
            if (grid.IsWithinBounds(x, newY) && grid.IsEmpty(x, newY))
            {
                grid.SwapElements(x, y, x, newY);
                return true; // Successful horizontal movement
            }
            blocked = true;
            return false;
        };

    //// Dispersion logic: Try spreading left and right up to the defined rate
    int maxDispersion = static_cast<int>(dispersionRate);
    for (int step = 1; step <= maxDispersion; ++step)
    {
        if (moveRightFirst)
        {
            if (tryHorizontal(step))
            {
                return; // Spread right
            }
            if (tryHorizontal(-step))
            {
                return;; // Spread left
            }
        }
        else
        {
            if (tryHorizontal(-step))
            {
                return; // Spread left
            }
            if (tryHorizontal(step))
            {
                return; // Spread right
            }
        }

        if (blocked) break;
    }

    // Step 4: If no movement occurred, mark as blocked
    blocked = true;
}

void ProcessSolid(Element* element, int x, int y, Grid& grid, bool& blocked)
{
    // Check if the element below (downwards) is empty or has a liquid component
    if (x < grid.GetRows() - 1 && (grid.IsEmpty(x + 1, y) || HasComponent<LiquidComp>(grid.GetElementData(x + 1, y), "Liquid")))
    {
        grid.SwapElements(x, y, x + 1, y); // Move down
    }
    else
    {
        // Randomize horizontal preference to simulate natural spread
        bool moveRightFirst = (rand() % 2 == 0);

        // Check movement directions: down-left, down-right
        auto tryMove = [&](int dx, int dy) {
            if (x < grid.GetRows() - 1 && y + dy >= 0 && y + dy < grid.GetColumns())
            {
                if (grid.IsEmpty(x + 1, y + dy) || HasComponent<LiquidComp>(grid.GetElementData(x + 1, y + dy), "Liquid"))
                {
                    grid.SwapElements(x, y, x + 1, y + dy); // Move diagonally
                    return true;
                }
            }
            return false;
            };

        // Try down-left, then down-right based on the random flag
        if (moveRightFirst)
        {
            if (!tryMove(-1, -1)) // down-left
                blocked = true; // No movement
            else if (!tryMove(1, 1)) // down-right
                blocked = true;
        }
        else
        {
            if (!tryMove(1, 1)) // down-right
                blocked = true; // No movement
            else if (!tryMove(-1, -1)) // down-left
                blocked = true;
        }
    }
}

#endif // !SYSTEMS_H