#ifndef SYSTEMS_H
#define SYSTEMS_H

#include "Utils.h"

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

void UpdateMovableSolid(Element* element, int x, Grid& grid, int y);

void UpdateLiquid(Element* element, int x, Grid& grid, int y);

void UpdateGas(Element* element, int x, Grid& grid, int y);

void UpdateGridElements(Grid& grid, float deltaTime)
{
    for (int x = grid.GetRows() - 1; x >= 0; --x) 
    {
        // Determine iteration direction based on the frame count
        int startY = grid.IsEvenFrame() ? 0 : grid.GetColumns() - 1;
        int endY = grid.IsEvenFrame() ? grid.GetColumns() : -1;
        int step = grid.IsEvenFrame() ? 1 : -1;

        for (int y = startY; y != endY; y += step)
        {
            if (grid.IsEmpty(x, y)) 
            {
                continue; // Skip empty cells
            }

            Element* element = grid.GetElementData(x, y);
            if(element->hasMoved) continue;

            // MOVABLE SOLIDS
            UpdateMovableSolid(element, x, grid, y);

            // LIQUIDS
            UpdateLiquid(element, x, grid, y);

            // GAS
            UpdateGas(element, x, grid, y);

            element->hasMoved = true;
        }
    }

    // reset dirty flag
    for (int x = grid.GetRows() - 1; x >= 0; --x)
    {
        for (int y = 0; y < grid.GetColumns(); ++y)
        {
            if (grid.IsEmpty(x, y))
            {
                continue; // Skip empty cells
            }

            Element* element = grid.GetElementData(x, y);
            element->hasMoved = false;
        }
    }
}

void UpdateGas(Element* element, int x, Grid& grid, int y)
{
    if (auto* gasComp = GetComponent<GasComp>(element, "Gas"))
    {
        // Apply inverse gravity logic for gas test
        if (x > 0 && (grid.IsEmpty(x - 1, y)))
        {
            grid.SwapElements(x, y, x - 1, y);
        }
        else if (x > 0 && y > 0 && grid.IsEmpty(x - 1, y - 1)) // move down left
        {
            grid.MoveElement(x, y, x - 1, y - 1);
        }
        else if (x > 0 && y < grid.GetColumns() - 1 && grid.IsEmpty(x - 1, y + 1)) // move down right
        {
            grid.MoveElement(x, y, x - 1, y + 1);
        }
        else if (y < grid.GetColumns() - 1 && grid.IsEmpty(x, y + 1)) // move right
        {
            grid.MoveElement(x, y, x, y + 1);
        }
        else if (y > 0 && grid.IsEmpty(x, y - 1)) // move left
        {
            grid.MoveElement(x, y, x, y - 1);
        }
    }
}

void UpdateLiquid(Element* element, int x, Grid& grid, int y)
{
    if (auto* movableComp = GetComponent<LiquidComp>(element, "Liquid"))
    {
        // Apply gravity logic for Liquid
        if (x < grid.GetRows() - 1 && grid.IsEmpty(x + 1, y))
        {
            grid.MoveElement(x, y, x + 1, y);
        }
        else if (x < grid.GetRows() - 1 && y > 0 && grid.IsEmpty(x + 1, y - 1)) // move down left
        {
            grid.MoveElement(x, y, x + 1, y - 1);
        }
        else if (x < grid.GetRows() - 1 && y < grid.GetColumns() - 1 && grid.IsEmpty(x + 1, y + 1)) // move down right
        {
            grid.MoveElement(x, y, x + 1, y + 1);
        }
        else
        {
            // Randomize horizontal preference to simulate natural liquid spread
            bool moveRightFirst = (rand() % 2 == 0);

            if (moveRightFirst)
            {
                if (y < grid.GetColumns() - 1 && grid.IsEmpty(x, y + 1)) // Try move right
                {
                    grid.MoveElement(x, y, x, y + 1);
                }
                else if (y > 0 && grid.IsEmpty(x, y - 1)) // Fallback: move left
                {
                    grid.MoveElement(x, y, x, y - 1);
                }
            }
            else
            {
                if (y > 0 && grid.IsEmpty(x, y - 1)) // Try move left
                {
                    grid.MoveElement(x, y, x, y - 1);
                }
                else if (y < grid.GetColumns() - 1 && grid.IsEmpty(x, y + 1)) // Fallback: move right
                {
                    grid.MoveElement(x, y, x, y + 1);
                }
            }
        }
    }
}

void UpdateMovableSolid(Element* element, int x, Grid& grid, int y)
{
    if (auto* movableComp = GetComponent<MovableSolidComp>(element, "MovableSolid"))
    {
        // Apply gravity logic for MovableSolid
        if (x < grid.GetRows() - 1 && (grid.IsEmpty(x + 1, y) || GetComponent<LiquidComp>(grid.GetElementData(x + 1, y), "Liquid")))
        {
            grid.SwapElements(x, y, x + 1, y);
        }
        else if (x < grid.GetRows() - 1 && y > 0 && (grid.IsEmpty(x + 1, y - 1) || GetComponent<LiquidComp>(grid.GetElementData(x + 1, y - 1), "Liquid"))) // move down left
        {
            grid.SwapElements(x, y, x + 1, y - 1);
        }
        else if (x < grid.GetRows() - 1 && y < grid.GetColumns() - 1 && (grid.IsEmpty(x + 1, y + 1) || GetComponent<LiquidComp>(grid.GetElementData(x + 1, y + 1), "Liquid"))) // move down right
        {
            grid.SwapElements(x, y, x + 1, y + 1);
        }
    }
}

void HeatSystem(Grid& grid, float deltaTime)
{
    //for (int x = grid.GetRows() - 1; x >= 0; --x)
    //{
    //    for (int y = 0; y < grid.GetColumns(); ++y)
    //    {
    //        ElementID id = grid.GetElementID(x, y);
    //        if (id != EMPTY_CELL)
    //        {
    //            const Element& element = grid.GetElementData(id);

    //            // Check if the element has a HeatSourceComp
    //            if (element.definition->components.count("HeatSource"))
    //            {
    //                auto& heatComp = std::get<HeatSourceComp>(element.definition->components.at("HeatSource"));

    //                // Apply heat to neighboring cells
    //                for (int dy = -1; dy <= 1; ++dy)
    //                {
    //                    for (int dx = -1; dx <= 1; ++dx)
    //                    {
    //                        if (dx == 0 && dy == 0) continue;  // Skip self
    //                        int nx = x + dx;
    //                        int ny = y + dy;

    //                        if (grid.IsWithinBounds(nx, ny))
    //                        {
    //                            ElementID neighborId = grid.GetElementID(nx, ny);
    //                            if (neighborId != EMPTY_CELL)
    //                            {
    //                                Element& neighbor = grid.GetElementData(neighborId);

    //                                // Check if the neighbor is flammable
    //                                if (neighbor.definition->components.count("Flammable"))
    //                                {
    //                                    auto& flammableComp = std::get<FlammableComp>(neighbor.definition->components.at("Flammable"));
    //                                    if (heatComp.temperature >= flammableComp.ignitionTemp)
    //                                    {
    //                                        // Set neighbor on fire or change its state
    //                                        //grid.SetElementOnFire(nx, ny);
    //                                    }
    //                                }
    //                            }
    //                        }
    //                    }
    //                }
    //            }
    //        }
    //    }
    //}
}


#endif // !SYSTEMS_H