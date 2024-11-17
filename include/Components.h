#ifndef COMPONENTS_H
#define COMPONENT_H

// Components are just data for elements
// systems will use this data to give actual functionality

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

struct FlammableComp
{
    float ignitionTemp{}; // Temperature at which the element ignites
    float burnRate{};     // How quickly the element burns
};

#endif // !COMPONENTS_H