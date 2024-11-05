#ifndef ELEMENT_H
#define ELEMENT_H

#include <string>
#include <memory>
#include "Behavior.h"
#include <glm.hpp>
#include <unordered_map>
#include <typeindex>

class Grid;
class Behavior;

class Element final
{

public:
	Element(const std::string& name, const glm::vec3& color) : m_Name{ name }, m_Color{ color } {}
    ~Element();

	Element(const Element& other) = delete;
	Element& operator=(const Element& other) = delete;
	Element(Element&& other) = default;
	Element& operator=(Element&& other) = default;
public:
	void Render() const;

    void Update(Grid& grid, int x, int y)
    {
        for (auto& behavior : m_pBehaviors)
        {
            // Pass both the current and next grid to each behavior so it can act accordingly
            behavior->Update(*this, grid, x, y);
        }
    }

    template <typename T, typename... Args>
    void AddBehavior(Args&&... args) 
    {
        m_pBehaviors.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    }

    template <typename T>
    T& GetBehavior() {
        for (auto& behavior : m_pBehaviors)
        {
            if (auto casted = std::dynamic_pointer_cast<T>(behavior))
            {
                return *casted;
            }
        }
        //throw std::runtime_error("Behavior not found!");
    }

    template <typename T>
    bool HasBehavior() const 
    {
        for (const auto& behavior : m_pBehaviors)
        {
            if (std::dynamic_pointer_cast<T>(behavior)) 
            {
                return true;
            }
        }
        return false;
    }

    const glm::vec3& GetColor() const
    {
        return m_Color;
    }
private:
	std::string m_Name{};
	glm::vec3 m_Color{};
	std::vector<std::unique_ptr<Behavior>> m_pBehaviors{};
public:
    bool m_IsUpdated{};
};

#endif // !ELEMENT_H