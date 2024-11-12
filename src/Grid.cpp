#include "Grid.h"
#include <SDL2/SDL.h>
#include <thread>
#include "InputManager.h"
#include "ServiceLocator.h"
#include <Systems.h>

Grid::Grid(const GridInfo& gridInfo)
    : m_GridInfo(gridInfo), m_pElementRegistry(std::make_unique<ElementRegistry>())
{
    m_Elements.resize(gridInfo.rows, std::vector<ElementID>(gridInfo.columns, EMPTY_CELL));
}

Grid::~Grid()
{
}

void Grid::Init()
{
    AddElementAt(0, 0, "Sand");
    AddElementAt(0, 1, "Sand");
    AddElementAt(1, 0, "Water");
}

inline ElementID Grid::GetElementID(int x, int y) const
{
    return m_Elements[x][y];
}

inline bool Grid::IsWithinBounds(int x, int y) const
{
    return x >= 0 && x < m_GridInfo.rows && y >= 0 && y < m_GridInfo.columns;
}

inline bool Grid::IsEmpty(int x, int y) const
{
    return GetElementID(x, y) == EMPTY_CELL;
}

void Grid::MoveElement(int x, int y, int newX, int newY)
{
    m_Elements[newX][newY] = m_Elements[x][y];
    m_Elements[x][y] = EMPTY_CELL;
}

inline const Element* Grid::GetElementData(int x, int y) const
{
    ElementID id = GetElementID(x, y);
    return id != EMPTY_CELL ? m_pElementRegistry->GetElementData(id) : nullptr;
}

void Grid::AddElementAt(int x, int y, const std::string& elementTypeName)
{
    if (IsWithinBounds(x, y) && IsEmpty(x, y))
    {
        ElementID id = m_pElementRegistry->AddElement(elementTypeName);
        m_Elements[x][y] = id;
    }
}

void Grid::RemoveElementAt(int x, int y)
{
    if (IsWithinBounds(x, y) && !IsEmpty(x, y))
    {
        ElementID id = m_Elements[x][y];
        m_pElementRegistry->RemoveElement(id);
        m_Elements[x][y] = EMPTY_CELL;
    }
}

void Grid::Update()
{
    UpdateSelection();

    if (InputManager::GetInstance().IsMouseButtonHeld(SDL_BUTTON_LEFT))
    {
        for (const auto& cell : GetSelectedCells())
        {
            
        }
    }

    if (InputManager::GetInstance().IsMouseButtonHeld(SDL_BUTTON_RIGHT))
    {
        for (const auto& cell : GetSelectedCells())
        {
            
        }
    }

    if (InputManager::GetInstance().IsScrolledUp())
    {
        ++m_SelectionBrushSize;
    }
    if (InputManager::GetInstance().IsScrolledDown())
    {
        if (m_SelectionBrushSize - 1 >= 1)
        {
            --m_SelectionBrushSize;
        }
    }
}

void Grid::FixedUpdate()
{
    UpdateElements();
}

void Grid::Render(Window* window) const
{
    RenderGrid(window);
    RenderElements(window);
    RenderSelection(window);
}


void Grid::UpdateElements()
{
    MovementSystem(*this, 0.f);
    HeatSystem(*this, 0.f);
}

void Grid::UpdateSelection()
{
    glm::vec2 mousePos = InputManager::GetInstance().GetMousePos();
    auto mouseGridPos = ConvertScreenToGrid(mousePos);

    if (IsWithinBounds(mouseGridPos.y, mouseGridPos.x))
    {
        m_MouseIsInGrid = true;
        m_SelectedCell = mouseGridPos;

        // Clear the main selection vector
        m_SelectedCells.clear();

        if (m_SelectionBrushSize <= 1)
        {
            return;
        }

        // Calculate the radius in cells directly from m_SelectionBrushSize
        int radiusInCells = static_cast<int>(m_SelectionBrushSize - 1);  // Now it's directly in cells

        // Temporary boolean grid to track selected cells
        std::vector<std::vector<bool>> tempSelectedGrid(m_GridInfo.rows, std::vector<bool>(m_GridInfo.columns, false));

        // Center of the selection brush in grid coordinates
        glm::vec2 center = { m_SelectedCell.x + 0.5f, m_SelectedCell.y + 0.5f };

        // Loop through all cells in the square defined by the selection brush
        for (int dx = -radiusInCells; dx <= radiusInCells; ++dx)
        {
            for (int dy = -radiusInCells; dy <= radiusInCells; ++dy)
            {
                // Directly calculate the position of the cell being checked
                glm::ivec2 cellPos = { m_SelectedCell.x + dx, m_SelectedCell.y + dy };

                // Check bounds first
                if (!IsWithinBounds(cellPos.y, cellPos.x)) continue;

                // Center of the current cell
                glm::vec2 cellCenter = { cellPos.x + 0.5f, cellPos.y + 0.5f };

                // Calculate the squared distance from the center of the selection to the cell center
                float dxSquared = (center.x - cellCenter.x) * (center.x - cellCenter.x);
                float dySquared = (center.y - cellCenter.y) * (center.y - cellCenter.y);
                float distanceSquared = dxSquared + dySquared;

                // Check if the distance is within the radius (squared to avoid sqrt)
                if (distanceSquared <= (radiusInCells * radiusInCells))
                {
                    // Check if the cell was already selected
                    if (!tempSelectedGrid[cellPos.y][cellPos.x])
                    {
                        // If valid, add to the selected cells
                        m_SelectedCells.push_back(cellPos);
                        tempSelectedGrid[cellPos.y][cellPos.x] = true;  // Mark as selected
                    }
                }
            }
        }
    }
    else
    {
        m_MouseIsInGrid = false;
        m_SelectedCells.clear();
    }
}

void Grid::RenderSelection(Window* window) const
{
    if (!m_MouseIsInGrid) return;

    if (m_SelectionBrushSize <= 1)
    {
        RenderDrawCell(window, m_SelectedCell);
        return;
    }


    for (const glm::ivec2& selectedCell : m_SelectedCells)
    {
        RenderDrawCell(window, selectedCell);
    }
}

void Grid::RenderDrawCell(Window* window, const glm::ivec2& selectedCell) const
{
    SDL_SetRenderDrawColor(window->GetRenderer(),
        static_cast<Uint8>(m_SelectionColor.r),
        static_cast<Uint8>(m_SelectionColor.g),
        static_cast<Uint8>(m_SelectionColor.b),
        static_cast<Uint8>(m_SelectionColor.a));

    SDL_Rect rect;
    rect.x = m_GridInfo.pos.x + selectedCell.x * m_GridInfo.cellSize;
    rect.y = m_GridInfo.pos.y + selectedCell.y * m_GridInfo.cellSize;
    rect.w = m_GridInfo.cellSize;
    rect.h = m_GridInfo.cellSize;

    SDL_RenderDrawRect(window->GetRenderer(), &rect);
}

void Grid::RenderGrid(Window* window) const
{
    SDL_SetRenderDrawColor(window->GetRenderer(), 100, 0, 0, 255); // Red color for grid lines

    // Draw vertical grid lines
    for (int x = 0; x <= this->GetColumns(); ++x)
    {
        SDL_RenderDrawLine(window->GetRenderer(), m_GridInfo.pos.x + x * m_GridInfo.cellSize, m_GridInfo.pos.y, m_GridInfo.pos.x + x * m_GridInfo.cellSize, m_GridInfo.pos.y + m_GridInfo.rows * m_GridInfo.cellSize); // Vertical line
    }

    // Draw horizontal grid lines
    for (int y = 0; y <= this->GetRows(); ++y)
    {
        SDL_RenderDrawLine(window->GetRenderer(), m_GridInfo.pos.x, m_GridInfo.pos.y + y * m_GridInfo.cellSize, m_GridInfo.pos.x + m_GridInfo.columns * m_GridInfo.cellSize, m_GridInfo.pos.y + y * m_GridInfo.cellSize); // Horizontal line
    }
}

void Grid::RenderElements(Window* window) const
{
    for (int x = 0; x < this->GetRows(); ++x)
    {
        for (int y = 0; y < this->GetColumns(); ++y)
        {
            if (!IsEmpty(x, y))
            {
                // Get the color as a uint32_t from the element's definition
                const Element* element = GetElementData(x, y);
                uint32_t color = element->definition->color;

                // Extract RGB components from the uint32_t color
                Uint8 r = (color >> 16) & 0xFF; // Extract red component
                Uint8 g = (color >> 8) & 0xFF;  // Extract green component
                Uint8 b = color & 0xFF;         // Extract blue component

                // Set the render draw color
                SDL_SetRenderDrawColor(window->GetRenderer(), r, g, b, 255);

                // Set up the rectangle for the cell position
                SDL_Rect rect;
                rect.x = m_GridInfo.pos.x + y * m_GridInfo.cellSize;
                rect.y = m_GridInfo.pos.y + x * m_GridInfo.cellSize;
                rect.w = m_GridInfo.cellSize;
                rect.h = m_GridInfo.cellSize;

                // Draw the filled rectangle for this element
                SDL_RenderFillRect(window->GetRenderer(), &rect);
            }
        }
    }
}


void Grid::ClearGrid()
{
    for (int x{}; x < this->GetRows(); ++x)
    {
        for (int y{}; y < this->GetColumns(); ++y)
        {
            RemoveElementAt(x, y);
        }
    }
}