#include "Grid.h"
#include <SDL.h>

Grid::Grid(const GridInfo& gridInfo)
	:
    m_GridInfo{ gridInfo },
	m_Cells(static_cast<size_t>(gridInfo.columns), std::vector<Cell>(static_cast<size_t>(gridInfo.rows)))
{
}

Grid::~Grid()
{
}

void Grid::Update()
{
 //   for (int x = 0; x < GetColumns(); ++x) 
 //   {
	//    for (int y = 0; y < GetRows(); ++y) 
 //       {
	//		auto& element = GetCell(x, y).m_pElement;
	//		if (element)
 //           {
	//			element->Update(*this, x, y); // Update based on behaviors
	//		}
	//	}
	//}

    for (int x = 0; x < GetColumns(); ++x)
    {
        for (int y = 0; y < GetRows(); ++y)
        {
            auto& element = GetCell(x, y).m_pElement;
            if (element)
            {
                element->Update(*this, x, y); // Update based on behaviors
            }
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

    for (int x{}; x < this->GetColumns(); ++x)
    {
        for (int y{}; y < this->GetRows(); ++y)
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