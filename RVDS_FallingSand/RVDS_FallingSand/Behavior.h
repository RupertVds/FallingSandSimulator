#ifndef BEHAVIOR_H
#define BEHAVIOR_H

class Element;
class Grid;

class Behavior
{
public:
    ~Behavior();
    virtual void Update(Element& element, Grid& grid, int x, int y) = 0;
};

// Movable solid component (for elements like sand)
class MovableSolid : public Behavior {
public:
    float mass;
    float inertia;

    MovableSolid(float mass, float inertia);

    void Update(Element& element, Grid& grid, int x, int y) override;
};

//// Fluid component (for elements like water)
//class Fluid : public Behavior {
//public:
//    float density;
//    float flowRate;
//
//    Fluid(float density, float flowRate) : density(density), flowRate(flowRate) {}
//
//    void Update(Element& element, Grid& grid, int x, int y) override {
//        // Example: Flow sideways if the cell is empty
//        if (grid.IsWithinBounds(x, y + 1) && grid.GetCell(x, y + 1).IsEmpty()) {
//            grid.MoveElement(x, y, x, y + 1);
//        }
//        else if (grid.IsWithinBounds(x - 1, y) && grid.GetCell(x - 1, y).IsEmpty()) {
//            grid.MoveElement(x, y, x - 1, y);
//        }
//        else if (grid.IsWithinBounds(x + 1, y) && grid.GetCell(x + 1, y).IsEmpty()) {
//            grid.MoveElement(x, y, x + 1, y);
//        }
//    }
//};

//// Heat source component (for elements like fire or lava)
//class HeatSource : public Behavior {
//public:
//    float temperature;
//    float heatTransferRate;
//
//    HeatSource(float temp, float rate) : temperature(temp), heatTransferRate(rate) {}
//
//    void Update(Element& element, Grid& grid, int x, int y) override
//    {
//        //// Example: Spread heat to neighboring cells
//        //for (int dx = -1; dx <= 1; ++dx) {
//        //    for (int dy = -1; dy <= 1; ++dy) {
//        //        if (dx != 0 || dy != 0) {
//        //            int nx = x + dx, ny = y + dy;
//        //            if (grid.IsWithinBounds(nx, ny)) {
//        //                auto& neighbor = grid.GetCell(nx, ny).m_pElement;
//        //                if (neighbor && neighbor->HasBehavior<Heatable>()) {
//        //                    neighbor->GetBehavior<Heatable>().increaseTemperature(temperature * heatTransferRate);
//        //                }
//        //            }
//        //        }
//        //    }
//        //}
//    }
//};

#endif // !BEHAVIOR_H
