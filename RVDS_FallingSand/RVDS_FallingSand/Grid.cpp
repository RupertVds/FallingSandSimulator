#include "Grid.h"
#include <SDL.h>
#include <thread>
#include "InputManager.h"

Grid::Grid(const GridInfo& gridInfo)
	:
    m_GridInfo{ gridInfo }
{
    m_CurrentGrid.resize(gridInfo.rows);
    m_NextGrid.resize(gridInfo.rows);

    for (int i = 0; i < gridInfo.rows; ++i) 
    {
        m_CurrentGrid[i].resize(gridInfo.columns);
        m_NextGrid[i].resize(gridInfo.columns);

        for (int j = 0; j < gridInfo.columns; ++j) 
        {
            m_CurrentGrid[i][j] = std::make_unique<Cell>();
            m_NextGrid[i][j] = std::make_unique<Cell>();
        }
    }


}

Grid::~Grid()
{
}

void Grid::Update()
{
    UpdateSelection();

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
    if (InputManager::GetInstance().IsMouseButtonHeld(SDL_BUTTON_LEFT))
    {
        for (const auto& cell : GetSelectedCells())
        {
            auto sand = std::make_unique<Element>("Sand", glm::vec3{ 194,178,128 });
            sand->AddBehavior<MovableSolid>(5.0f, 0.2f);

            // maybe only if empty otherwise replace?
            assert(x >= m_GridInfo.rows);
            assert(y >= m_GridInfo.columns);
            GetCurrentGrid()[cell.y][cell.x]->m_pElement.reset();
            GetCurrentGrid()[cell.y][cell.x]->m_pElement = std::move(sand);
        }
    }

    UpdateElements();



    // Swap buffers at the end of the update
    SwapBuffers();
    ClearNextGrid();  // Reset the next grid for the next update
}

void Grid::Render(Window* window) const
{
    RenderGrid(window);
    RenderElements(window);
    RenderSelection(window);
}


void Grid::UpdateElements()
{
    for (int x = GetRows() - 1; x >= 0; --x) {
        for (int y = 0; y < GetColumns(); ++y) {
            auto& cell = m_CurrentGrid[x][y];

            if (cell->m_pElement) {
                // Update element and place it in the next grid
                cell->m_pElement->Update(*this, x, y);
            }
        }
    }
}

void Grid::UpdateSelection()
{
    glm::vec2 mousePos = InputManager::GetInstance().GetMousePos();
    auto mouseGridPos = ConvertScreenToGrid(mousePos);

    if (IsWithinBounds(mouseGridPos.y, mouseGridPos.x))
    {
        m_MouseIsInGrid = true;
        m_SelectedCell = mouseGridPos;

        // Calculate the radius in cells directly from m_SelectionBrushSize
        int radiusInCells = static_cast<int>(m_SelectionBrushSize - 1);  // Now it's directly in cells

        // Temporary boolean grid to track selected cells
        std::vector<std::vector<bool>> tempSelectedGrid(m_GridInfo.rows, std::vector<bool>(m_GridInfo.columns, false));

        // Clear the main selection vector
        m_SelectedCells.clear();

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


    for (const glm::ivec2& selectedCell : m_SelectedCells)
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
            if (!m_CurrentGrid[x][y]->IsEmpty())
            {
                glm::vec3 color = m_CurrentGrid[x][y]->m_pElement->GetColor();
                SDL_SetRenderDrawColor(window->GetRenderer(),
                    static_cast<Uint8>(color.r),
                    static_cast<Uint8>(color.g),
                    static_cast<Uint8>(color.b),
                    static_cast<Uint8>(255));

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

void Grid::ClearGrid()
{
    for (int x{}; x < this->GetRows(); ++x)
    {
        for (int y{}; y < this->GetColumns(); ++y)
        {
            m_CurrentGrid[x][y]->m_pElement.reset();
        }
    }
}

void Grid::ClearNextGrid()
{
    for (int x{}; x < this->GetRows(); ++x)
    {
        for (int y{}; y < this->GetColumns(); ++y)
        {
            m_NextGrid[x][y]->m_pElement.reset();
        }
    }
}

void Grid::SwapBuffers()
{
    std::swap(m_CurrentGrid, m_NextGrid);
}
