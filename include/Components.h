#ifndef COMPONENTS_H
#define COMPONENT_H
#include <string>
// Components are just data for elements
// systems will use this data to give actual functionality
// this data cannot be changed since it is used a flyweight any data that needs to change
// needs to be stored inside the element class; thus we want as least as possible

struct SolidComp
{
    float density{};
};

struct LiquidComp
{
    float density{};
    float dispersionRate{}; // Horizontal spread range
};

struct GasComp
{
    float density{};
    float riseRate{}; // Upward velocity
};

struct GravityComp
{
    float gravityScale{}; // Strength of gravity effect
};

struct SpreadableComp
{
    float spreadThreshold{}; // spreadfactor at which the element gets taken over by the spreading element (like fire)
    float spreadResistance{}; // how many update steps a spreading component needs to spread to this
};

struct SpreadingComp
{
    float spreadFactor{};
    float spreadChance{};
};

struct LifeTimeComp
{
    float maxLifeTime{}; // -1 for infinite
    std::string elementToSpawn{};
};

#endif // !COMPONENTS_H