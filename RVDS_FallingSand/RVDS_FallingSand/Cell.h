#ifndef CELL_H
#define CELL_H

#include <memory>

#include "Element.h"

class Element;

class Cell final
{
public:
    Cell() : m_pElement{ nullptr } {}
    ~Cell();

    std::shared_ptr<Element> m_pElement{};

    bool IsEmpty() const { return !m_pElement; }
};

#endif // !CELL_H
