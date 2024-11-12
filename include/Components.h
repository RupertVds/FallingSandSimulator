#ifndef COMPONENTS_H
#define COMPONENT_H

// Components are just data for elements
// systems will use this data to give actual functionality

struct MovableSolidComp final
{
	float mass{ 1.f }; // multiplied by gravity to know how fast it should fall
	float friction{ 1.f }; // slowdown once we fall on the ground
};

struct LiquidComp final
{
	float viscosity{ 1.f };
	float flowRate{ 0.5f };
};

struct FlammableComp final
{
	float ignitionTemp{ 1.f }; // temperature at which the element turns into fire
};

struct HeatSourceComp final
{
	float temperature{ 1.f }; // temperature at which the element affects nearby elements
};

#endif // !COMPONENTS_H