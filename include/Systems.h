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

void MovementSystem(Grid& grid, float deltaTime) 
{
    for (int x = grid.GetRows() - 1; x >= 0; --x) 
    {
        for (int y = 0; y < grid.GetColumns(); ++y)
        {
            if (grid.IsEmpty(x, y)) 
            {
                continue; // Skip empty cells
            }

            Element* element = grid.GetElementData(x, y);
            if(element->hasMoved) continue;

            // MOVABLE SOLIDS

            if (auto* movableComp = GetComponent<MovableSolidComp>(element, "MovableSolid"))
            {
                // Apply gravity logic for MovableSolid
                if (x < grid.GetRows() - 1 && (grid.IsEmpty(x + 1, y) || GetComponent<LiquidComp>(grid.GetElementData(x + 1, y), "Liquid")))
                {
                    grid.SwapElements(x, y, x + 1, y);
                }
                else if (x < grid.GetRows() - 1 && y > 0 && grid.IsEmpty(x + 1, y - 1)) // move down left
                {
                    grid.MoveElement(x, y, x + 1, y - 1);
                }
                else if (x < grid.GetRows() - 1 && y < grid.GetColumns() - 1 && grid.IsEmpty(x + 1, y + 1)) // move down right
                {
                    grid.MoveElement(x, y, x + 1, y + 1);
                }
            }

            // GAS
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


            // LIQUIDS
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
                else if (y < grid.GetColumns() - 1 && grid.IsEmpty(x, y + 1)) // move right
                {
                    grid.MoveElement(x, y, x, y + 1);
                }
                else if (y > 0 && grid.IsEmpty(x, y - 1)) // move left
                {
                    grid.MoveElement(x, y, x, y - 1);
                }

            }

            element->hasMoved = true;
        }
    }

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
