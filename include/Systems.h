#ifndef SYSTEMS_H
#define SYSTEMS_H

// these are all systems that are applied on the components of the elements

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

            const Element* element = grid.GetElementData(x, y);
            if (element && element->definition->components.count("MovableSolid"))
            {
                auto& movableComp = std::get<MovableSolidComp>(element->definition->components.at("MovableSolid"));

                // Apply gravity logic
                // move down
                if (x < grid.GetRows() - 1 && grid.IsEmpty(x + 1, y)) 
                {
                    grid.MoveElement(x, y, x + 1, y);
                }
                else if (x < grid.GetRows() - 1 && y > 0 && grid.IsEmpty(x + 1, y - 1)) // move down left
                {
                    grid.MoveElement(x, y, x + 1, y - 1);
                }
                else if (x < grid.GetRows() - 1 && y < grid.GetColumns() && grid.IsEmpty(x + 1, y + 1)) // move down left
                {
                    grid.MoveElement(x, y, x + 1, y + 1);
                }
            }
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
