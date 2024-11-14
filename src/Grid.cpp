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
    AddElementAt(1, 1, "Water");
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
    const glm::ivec2 gridMousePos = ConvertScreenToGrid(InputManager::GetInstance().GetMousePos());

    if (InputManager::GetInstance().IsKeyHeld(SDL_SCANCODE_1))
    {
        if (gridMousePos != glm::ivec2{ -1, -1 })
        {
            AddElementAt(gridMousePos.x, gridMousePos.y, "Sand");
        }
    }

    if (InputManager::GetInstance().IsKeyHeld(SDL_SCANCODE_2))
    {
        if (gridMousePos != glm::ivec2{ -1, -1 })
        {
            AddElementAt(gridMousePos.x, gridMousePos.y, "Water");
        }
    }

    if (InputManager::GetInstance().IsKeyHeld(SDL_SCANCODE_0))
    {
        if (gridMousePos != glm::ivec2{ -1, -1 })
        {
            RemoveElementAt(gridMousePos.x, gridMousePos.y);
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
    RenderElements(window);
    RenderGrid(window);
    RenderBrush(window);
}

void Grid::UpdateElements()
{
    MovementSystem(*this, 0.f);
    HeatSystem(*this, 0.f);
}

void Grid::RenderBrush(Window* window) const
{
    if (!m_MouseIsInGrid) return;
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
    static SDL_Texture* gridTexture = nullptr;
    //static bool textureNeedsUpdate = true;

    // create texture if no texture
    if (!gridTexture)
    {
        gridTexture = SDL_CreateTexture(
            window->GetRenderer(),
            SDL_PIXELFORMAT_RGB888,
            SDL_TEXTUREACCESS_STREAMING,
            this->GetColumns(),
            this->GetRows()
        );
       // textureNeedsUpdate = true;
    }

    //if (textureNeedsUpdate)
    {
        void* pixels;
        int pitch;
        SDL_LockTexture(gridTexture, nullptr, &pixels, &pitch);

        Uint32* pixelData = static_cast<Uint32*>(pixels);


        for (int x = 0; x < this->GetRows(); ++x)
        {
            for (int y = 0; y < this->GetColumns(); ++y)
            {
                if (!IsEmpty(x, y))
                {
                    const Element* element = GetElementData(x, y);
                    uint32_t color = element->definition->color;
                    // Set the corresponding "pixel" in the texture
                    pixelData[x * (pitch / 4) + y] = color;
                }
                else
                {
                    // Set empty cells to a background color (optional)
                    pixelData[x * (pitch / 4) + y] = 0x1A1A1A; // Black with full opacity
                }
            }
        }

        SDL_UnlockTexture(gridTexture);

        //textureNeedsUpdate = false;
    }

    SDL_Rect destRect = {
        m_GridInfo.pos.x,
        m_GridInfo.pos.y,
        m_GridInfo.cellSize * this->GetColumns(),
        m_GridInfo.cellSize * this->GetRows() };

    SDL_RenderCopy(window->GetRenderer(), gridTexture, nullptr, &destRect);
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