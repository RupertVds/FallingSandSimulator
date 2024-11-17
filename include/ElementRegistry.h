#ifndef ELEMENTREGISTRY_H
#define ELEMENTREGISTRY_H

#include "Components.h"
#include <unordered_map>
#include <string>
#include <glm/glm.hpp>
#include <variant>


// I erase the type so we can store all kinds of components
using Component = std::variant<SolidComp, LiquidComp, GasComp, GravityComp, FlammableComp>;

struct ElementDefinition
{
	std::string name{};
	uint32_t color{};
	std::unordered_map<std::string, Component> components{};
};

using ElementID = uint32_t;			// Unique identifier for each element
constexpr ElementID EMPTY_CELL = 0;	// 0 will be used to indicate "empty" elements

struct Element
{
	const ElementDefinition* definition{}; // Pointer to the shared particle type definition
	glm::vec2 velocity{};
	bool hasMoved{};
	int8_t tint; // Tint adjustment (-128 to 127)
};

class ElementRegistry final
{
public:
	ElementRegistry();
	~ElementRegistry() = default;
	ElementRegistry(const ElementRegistry& other) = delete;
	ElementRegistry& operator=(const ElementRegistry& other) = delete;
	ElementRegistry(ElementRegistry&& other) = delete;
	ElementRegistry& operator=(ElementRegistry&& other) = delete;
public:
	ElementID AddElement(const std::string& elementTypeName);
	void RemoveElement(ElementID id);
	Element* GetElementData(ElementID id);
	const ElementDefinition& GetElementType(const std::string& name) const;

private:
	// map unique element ids to unique element data
	std::unordered_map<ElementID, Element> m_ElementData{};
	// flyweight pattern for element definitions
	std::unordered_map<std::string, ElementDefinition> m_ElementTypes{};
	ElementID m_NextElementID = 1; // start IDs from 1 (0 for EMPTY_CELL)
};

#endif // !ELEMENTREGISTRY_H
