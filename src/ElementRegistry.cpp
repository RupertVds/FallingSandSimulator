#include "ElementRegistry.h"
#include <iostream>

ElementRegistry::ElementRegistry() 
{
    ElementDefinition sand{ "Sand", 0xD2B48C, 
        {
        {"Solid", SolidComp{1.f}},
        {"Gravity", GravityComp{2.f}}
        } 
    };
    m_ElementTypes["Sand"] = sand;

    ElementDefinition water{ "Water", 0x3498DB,
        {
            {"Liquid", LiquidComp{0.1f, 15.f}},
            {"Gravity", GravityComp{2.f}}
        } 
    };
    m_ElementTypes["Water"] = water;

    ElementDefinition smoke{ "Smoke", 0x848884,
        {
            {"Gas", GasComp{5.f}},
            {"Lifetime", LifeTimeComp{5.f, 8.f, "Empty"}}
        }
    };
    m_ElementTypes["Smoke"] = smoke;

    ElementDefinition wall{ "Wall", 0x2b2a2a,
    {
    }
    };
    m_ElementTypes["Wall"] = wall;

    ElementDefinition wood{ "Wood", 0x784520,
    {
        {"Spreadable", SpreadableComp{0.1f, 3}},
        {"Solid", SolidComp{1.f}},
        {"Gravity", GravityComp{2.f}}
    }
    };
    m_ElementTypes["Wood"] = wood;

    //ElementDefinition fire{ "Fire", 0xde5f0b,
    ElementDefinition fire{ "Fire", 0xfc6908,
    {
        {"Spreading", SpreadingComp{5.f, 0.1f}},
        {"Lifetime", LifeTimeComp{1.f, 1.5f, "Smoke"}}
    }
    };
    m_ElementTypes["Fire"] = fire;
}

ElementID ElementRegistry::AddElement(const std::string& elementTypeName)
{
    ElementID id = m_NextElementID++;
    auto it = m_ElementTypes.find(elementTypeName);
    assert(it != m_ElementTypes.end() && "Element type not found! Ensure the type is correctly registered.");

    // If assertions are disabled
    if (it == m_ElementTypes.end())
    {
        std::cout << "Warning: Element \"" + elementTypeName + "\" not found! Returning EMPTY_CELL.\n";
        return EMPTY_CELL;
    }

    // Generate a random tint adjustment (-15 to +15)
    int8_t randomTint = static_cast<int8_t>((rand() % 31) - 15);

    m_ElementData[id] = { &m_ElementTypes[elementTypeName], glm::vec2{0.0f, 0.0f}, false, false, 0, randomTint };

    Element* element = GetElementData(id);

    if (element)
    {
        auto it = element->definition->components.find("Lifetime");
        if (it != element->definition->components.end())
        {
            const LifeTimeComp* lifetimeComp = std::get_if<LifeTimeComp>(&it->second); // Safely retrieve the component
            element->lifeTime = lifetimeComp->minLifeTime + static_cast<float>(rand()) / static_cast<float>(RAND_MAX / (lifetimeComp->maxLifeTime - lifetimeComp->minLifeTime));;
        }
    }
    return id;
}

void ElementRegistry::RemoveElement(ElementID id) 
{
    m_ElementData.erase(id);
}

Element* ElementRegistry::GetElementData(ElementID id)
{
    auto it = m_ElementData.find(id);
    return it != m_ElementData.end() ? &it->second : nullptr;
}

const ElementDefinition* ElementRegistry::GetElementType(const std::string& name) const
{
    auto it = m_ElementTypes.find(name);
    return it != m_ElementTypes.end() ? &it->second : nullptr;
}

const std::unordered_map<std::string, ElementDefinition>& ElementRegistry::GetElementTypes() const
{
    return m_ElementTypes;
}

void ElementRegistry::AddElementType(const ElementDefinition& definition)
{
    m_ElementTypes[definition.name] = definition;
}