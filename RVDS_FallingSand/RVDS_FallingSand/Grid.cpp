#include "Grid.h"
#include <SDL.h>
#include <thread>

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
    for (int x = GetRows() - 1; x >= 0; --x) // works the same
    //for (int x = 0; x < GetRows(); ++x) 
    {
	    for (int y = 0; y < GetColumns(); ++y) 
        {
			Cell& cell= GetCell(x, y);
            if (cell.m_IsUpdated)
            {
                return;
            }
			if (cell.m_pElement)
            {
                cell.m_pElement->Update(*this, x, y); // Update based on behaviors
                cell.m_IsUpdated = true;
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

void Grid::Render(Window* window) const
{
    // Get the window dimensions
    int windowWidth = window->GetColumns();
    int windowHeight = window->GetRows();

    SDL_SetRenderDrawColor(window->GetRenderer(), 100, 0, 0, 255); // Red color for grid lines

    // Draw vertical grid lines
    for (int x = 0; x <= this->GetColumns(); ++x) {
        SDL_RenderDrawLine(window->GetRenderer(), m_GridInfo.pos.x + x * m_GridInfo.cellSize, m_GridInfo.pos.y, m_GridInfo.pos.x + x * m_GridInfo.cellSize, m_GridInfo.pos.y + m_GridInfo.rows * m_GridInfo.cellSize); // Vertical line
    }

    // Draw horizontal grid lines
    for (int y = 0; y <= this->GetRows(); ++y) {
        SDL_RenderDrawLine(window->GetRenderer(), m_GridInfo.pos.x, m_GridInfo.pos.y + y * m_GridInfo.cellSize, m_GridInfo.pos.x + m_GridInfo.columns * m_GridInfo.cellSize, m_GridInfo.pos.y + y * m_GridInfo.cellSize); // Horizontal line
    }

    for (int x{}; x < this->GetRows(); ++x)
    {
        for (int y{}; y < this->GetColumns(); ++y)
        {
            if (!m_Cells[x][y].IsEmpty())
            {
                glm::vec3 color = m_Cells[x][y].m_pElement->GetColor();
                SDL_SetRenderDrawColor(window->GetRenderer(), color.r, color.g, color.b, 255); // Red color for grid lines

                SDL_Rect rect;
                rect.x = m_GridInfo.pos.x + x * m_GridInfo.cellSize;
                rect.y = m_GridInfo.pos.y + y * m_GridInfo.cellSize;
                rect.w = m_GridInfo.cellSize;
                rect.h = m_GridInfo.cellSize;


                SDL_RenderFillRect(window->GetRenderer(), &rect);
            }
        }

    }
}