#ifndef CELL_H
#define CELL_H

#include "Element.h"
#include <memory>

class Element;

class Cell final
{
public:
    Cell() : m_pElement{ nullptr } {}

    // Move constructor
    Cell(Cell&& other) noexcept : m_pElement(std::move(other.m_pElement)) {}

    // Move assignment operator
    Cell& operator=(Cell&& other) noexcept {
        if (this != &other) {
            m_pElement = std::move(other.m_pElement);
        }
        return *this;
    }

    //Cell(Cell&& other) noexcept = default;
    //Cell& operator=(Cell&& other) noexcept = default;
    Cell(Cell& other) = delete;
    Cell& operator=(const Cell& other) = delete;

    ~Cell();

    std::unique_ptr<Element> m_pElement{};

    bool IsEmpty() const { return !m_pElement; }
};

#endif // !CELL_H
