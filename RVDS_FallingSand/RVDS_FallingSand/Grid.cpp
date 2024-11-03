#include "Grid.h"
#include <SDL.h>
#include <thread>
#include "InputManager.h"

Grid::Grid(const GridInfo& gridInfo)
	:
    m_GridInfo{ gridInfo },
	m_Cells(static_cast<size_t>(gridInfo.rows), std::vector<Cell>(static_cast<size_t>(gridInfo.columns)))
{
}

Grid::~Grid()
{
}

void Grid::Update()
{
    UpdateElements();
    UpdateSelection();
}

void Grid::Render(Window* window) const
{
    RenderGrid(window);
    RenderElements(window);
    RenderSelection(window);
}

void Grid::UpdateElements()
{
    for (int x = GetRows() - 1; x >= 0; --x) // works the same
        //for (int x = 0; x < GetRows(); ++x) 
    {
        for (int y = 0; y < GetColumns(); ++y)
        {
            Cell& cell = GetCell(x, y);
            if (cell.m_IsUpdated)
            {
                return;
            }
            if (cell.m_pElement)
            {
                cell.m_pElement->Update(*this, x, y); // Update based on behaviors
                cell.m_IsUpdated = true;
                //std::cout << "Updated Cell: x =" + std::to_string(x) + " ,y = " + std::to_string(y) + "\n";
                //std::this_thread::sleep_for(std::chrono::seconds(static_cast<long long>(1)));
            }
        }
    }

    for (int x = 0; x < GetRows(); ++x)
    {
        for (int y = 0; y < GetColumns(); ++y)
        {
            m_Cells[x][y].m_IsUpdated = false;
        }
    }
}

void Grid::UpdateSelection()
{
    if (InputManager::GetInstance().IsMouseButtonPressed(SDL_BUTTON_RIGHT))
    {
        std::cout << "CLICKED\n";
        glm::vec2 mousePos = InputManager::GetInstance().GetMousePos();
        auto gridPos = ConvertScreenToGrid(mousePos);
        std::cout << "MOUSEPOS: X = " + std::to_string(mousePos.x) + " , Y = " + std::to_string(mousePos.y) + "\n";
        std::cout << "GRIDPOS: X = " + std::to_string(gridPos.x) + " , Y = " + std::to_string(gridPos.y) + "\n";
    }
}

void Grid::RenderSelection(Window* window) const
{
    glm::ivec2 mousePos = InputManager::GetInstance().GetMousePos();

    // Convert mouse position to grid coordinates
    glm::ivec2 gridPos = ConvertScreenToGrid(mousePos);

    // Check if the converted position is within grid bounds
    if (IsWithinBounds(gridPos.y, gridPos.x)) // gridPos.y is row (x), gridPos.x is column (y)
    {
        // Get the color of the element in the selected cell or set a default selection color
        //glm::vec3 color = (m_Cells[gridPos.y][gridPos.x].IsEmpty())
        //    ? glm::vec3(200, 200, 200) // Light gray for empty cell
        //    : m_Cells[gridPos.y][gridPos.x].m_pElement->GetColor();        
        
        glm::vec3 color = glm::vec3(200, 200, 200); // Light gray for empty cell

        SDL_SetRenderDrawColor(window->GetRenderer(), color.r, color.g, color.b, 150); // Semi-transparent

        if (InputManager::GetInstance().IsMouseButtonPressed(SDL_BUTTON_LEFT))
        {
            std::cout << "Gridpos: x,y= " + std::to_string(gridPos.y) + ", " + std::to_string(gridPos.x) + "\n";
        }

        SDL_Rect rect;
        rect.x = m_GridInfo.pos.x + gridPos.x * m_GridInfo.cellSize;
        rect.y = m_GridInfo.pos.y + gridPos.y * m_GridInfo.cellSize;
        rect.w = m_GridInfo.cellSize;
        rect.h = m_GridInfo.cellSize;

        SDL_RenderDrawRect(window->GetRenderer(), &rect);
        
    }
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
    for (int x{}; x < this->GetRows(); ++x)
    {
        for (int y{}; y < this->GetColumns(); ++y)
        {
            if (!m_Cells[x][y].IsEmpty())
            {
                glm::vec3 color = m_Cells[x][y].m_pElement->GetColor();
                SDL_SetRenderDrawColor(window->GetRenderer(), color.r, color.g, color.b, 255); // Red color for grid lines

                SDL_Rect rect;
                rect.x = m_GridInfo.pos.x + y * m_GridInfo.cellSize;
                rect.y = m_GridInfo.pos.y + x * m_GridInfo.cellSize;
                rect.w = m_GridInfo.cellSize;
                rect.h = m_GridInfo.cellSize;


                SDL_RenderFillRect(window->GetRenderer(), &rect);
            }
        }
    }
}