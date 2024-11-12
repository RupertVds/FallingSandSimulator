#include "ElementRegistry.h"

ElementRegistry::ElementRegistry() 
{
    ElementDefinition sand{ "Sand", 0xD2B48C, {{"MovableSolid", MovableSolidComp{0.3f, 1.0f}}} };
    m_ElementTypes["Sand"] = sand;

    ElementDefinition water{ "Water", 0x3498DB, 
        {
            {"Liquid", LiquidComp{0.1f, 0.5f}},
            {"MovableSolid", MovableSolidComp{0.3f, 1.0f}}
        } 
    };
    m_ElementTypes["Water"] = water;
}

ElementID ElementRegistry::AddElement(const std::string& elementTypeName)
{
    ElementID id = m_NextElementID++;
    m_ElementData[id] = { &m_ElementTypes[elementTypeName], glm::vec2{0.0f, 0.0f} };
    return id;
}

void ElementRegistry::RemoveElement(ElementID id) 
{
    m_ElementData.erase(id);
}

const Element* ElementRegistry::GetElementData(ElementID id) const
{
    auto it = m_ElementData.find(id);
    return it != m_ElementData.end() ? &it->second : nullptr;
}

const ElementDefinition& ElementRegistry::GetElementType(const std::string& name) const
{
    return m_ElementTypes.at(name);
}
