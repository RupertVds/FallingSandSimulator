#include "Grid.h"
#include <SDL2/SDL.h>
#include <thread>
#include "InputManager.h"
#include "ServiceLocator.h"
#include <Systems.h>
#include "Utils.h"

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
	m_ElementToDraw = "Sand";

	AddElementAt(0, 0, "Sand");
	AddElementAt(0, 1, "Sand");
	AddElementAt(1, 0, "Water");
	AddElementAt(1, 1, "Water");

	m_PreviousGridMousePos = ConvertScreenToGrid(InputManager::GetInstance().GetMousePos());
}

void Grid::Update()
{
	UpdateInput();
}

void Grid::UpdateInput()
{
	const glm::ivec2 gridMousePos = ConvertScreenToGrid(InputManager::GetInstance().GetMousePos());
	if (gridMousePos == glm::ivec2{ -1, -1 })
	{
		m_MouseIsInGrid = false;
		m_PreviousGridMousePos = gridMousePos;
		return;
	}
	m_MouseIsInGrid = true;

	// ALT CLICK TO SELECT NEW ELEMENT TYPE
	if (InputManager::GetInstance().IsKeyHeld(SDL_SCANCODE_LALT) && InputManager::GetInstance().IsMouseButtonPressed(SDL_BUTTON_LEFT))
	{
		const Element* element = GetElementData(gridMousePos.x, gridMousePos.y);
		m_ElementToDraw = element->definition->name;
	}
	// CLICK TO PLACE IT
	else if (InputManager::GetInstance().IsMouseButtonHeld(SDL_BUTTON_LEFT))
	{
		if (m_PreviousGridMousePos == glm::ivec2{ -1, -1 })
		{
			m_PreviousGridMousePos = gridMousePos;
		}

		BresenhamLine(m_PreviousGridMousePos, gridMousePos, [&](int x, int y)
			{
				AddElementBrushed(x, y, m_ElementToDraw);
			}
		);
	}

	if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_RETURN))
	{
		if (m_PreviousGridMousePos == glm::ivec2{ -1, -1 })
		{
			m_PreviousGridMousePos = gridMousePos;
		}

		BresenhamLine(m_PreviousGridMousePos, gridMousePos, [&](int x, int y)
			{
				AddElementBrushed(x, y, m_ElementToDraw);
			}
		);
	}

	if (InputManager::GetInstance().IsMouseButtonHeld(SDL_BUTTON_RIGHT))
	{
		if (m_PreviousGridMousePos == glm::ivec2{ -1, -1 })
		{
			m_PreviousGridMousePos = gridMousePos;
		}
		//RemoveElementAt(gridMousePos.x, gridMousePos.y);
		BresenhamLine(m_PreviousGridMousePos, gridMousePos, [&](int x, int y)
			{
				RemoveElementBrushed(x, y);
			}
		);
	}

	if (InputManager::GetInstance().IsScrolledUp())
	{
		++m_BrushSize;
	}
	if (InputManager::GetInstance().IsScrolledDown())
	{
		if (m_BrushSize - 1 >= 1)
		{
			--m_BrushSize;
		}
	}
	
	if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_0))
	{
		m_ElementToDraw = "Wall";
	}
	if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_1))
	{
		m_ElementToDraw = "Sand";
	}
	if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_2))
	{
		m_ElementToDraw = "Water";
	}
	if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_4))
	{
		m_ElementToDraw = "Smoke";
	}

	m_PreviousGridMousePos = gridMousePos;
}

void Grid::FixedUpdate()
{
	UpdateElements();
}

void Grid::UpdateElements()
{
	MovementSystem(*this, 0.f);
	HeatSystem(*this, 0.f);
}

void Grid::Render(Window* window) const
{
	RenderElements(window);
	RenderGrid(window);
	RenderBrush(window);
}

void Grid::RenderBrush(Window* window) const
{
	if (!m_MouseIsInGrid) return;

	// Get mouse position in grid space
	const glm::ivec2 gridMousePos = ConvertScreenToGrid(InputManager::GetInstance().GetMousePos());

	// Define grid properties
	int gridOriginX = m_GridInfo.pos.x; // X position of grid origin on screen
	int gridOriginY = m_GridInfo.pos.y; // Y position of grid origin on screen
	int cellSize = m_GridInfo.cellSize;       // Size of a single cell in pixels

	// Calculate the center of the brush in screen space
	int screenCenterX = gridOriginX + gridMousePos.x * cellSize + cellSize / 2;
	int screenCenterY = gridOriginY + gridMousePos.y * cellSize + cellSize / 2;

	// Circle properties
	float radius = m_BrushSize * cellSize; // Radius in pixels
	int segments = 100;                    // Number of segments to approximate the circle

	// Set the draw color
	SDL_SetRenderDrawColor(window->GetRenderer(), 255, 255, 255, 128);

	// Calculate and draw the circle using line segments
	for (int i = 0; i < segments; ++i)
	{
		float theta1 = (2.0f * M_PI * i) / segments;
		float theta2 = (2.0f * M_PI * (i + 1)) / segments;

		int x1 = static_cast<int>(screenCenterX + radius * cos(theta1));
		int y1 = static_cast<int>(screenCenterY + radius * sin(theta1));

		int x2 = static_cast<int>(screenCenterX + radius * cos(theta2));
		int y2 = static_cast<int>(screenCenterY + radius * sin(theta2));

		SDL_RenderDrawLine(window->GetRenderer(), y1, x1, y2, x2);
	}
}

void Grid::RenderGrid(Window* window) const
{
	//return;
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

inline ElementID Grid::GetElementID(int x, int y) const
{
	return m_Elements[x][y];
}

inline Element* Grid::GetElementData(int x, int y) const
{
	ElementID id = GetElementID(x, y);
	return id != EMPTY_CELL ? m_pElementRegistry->GetElementData(id) : nullptr;
}

inline bool Grid::IsWithinBounds(int x, int y) const
{
	return x >= 0 && x < m_GridInfo.rows && y >= 0 && y < m_GridInfo.columns;
}

inline bool Grid::IsEmpty(int x, int y) const
{
	return GetElementID(x, y) == EMPTY_CELL;
}

void Grid::AddElementBrushed(int x, int y, const std::string& elementTypeName)
{
	float radius = m_BrushSize - 0.5f; // Fractional brush size

	// Calculate the bounding box of the circle
	int minX = static_cast<int>(std::floor(x - radius));
	int maxX = static_cast<int>(std::ceil(x + radius));
	int minY = static_cast<int>(std::floor(y - radius));
	int maxY = static_cast<int>(std::ceil(y + radius));

	// Iterate over all cells in the bounding box
	for (int i = minX; i <= maxX; ++i)
	{
		for (int j = minY; j <= maxY; ++j)
		{
			// Check if the cell is within the circle
			float distance = std::sqrt((i - x) * (i - x) + (j - y) * (j - y));
			if (distance <= radius && IsWithinBounds(i, j) && IsEmpty(i, j))
			{
				AddElementAt(i, j, elementTypeName);
			}
		}
	}
}

void Grid::AddElementAt(int x, int y, const std::string& elementTypeName)
{
	if (IsWithinBounds(x, y) && IsEmpty(x, y))
	{
		ElementID id = m_pElementRegistry->AddElement(elementTypeName);
		m_Elements[x][y] = id;
	}
}

void Grid::RemoveElementBrushed(int x, int y)
{
	float radius = m_BrushSize - 0.5f; // Fractional brush size

	// Calculate the bounding box of the circle
	int minX = static_cast<int>(std::floor(x - radius));
	int maxX = static_cast<int>(std::ceil(x + radius));
	int minY = static_cast<int>(std::floor(y - radius));
	int maxY = static_cast<int>(std::ceil(y + radius));

	// Iterate over all cells in the bounding box
	for (int i = minX; i <= maxX; ++i)
	{
		for (int j = minY; j <= maxY; ++j)
		{
			// Check if the cell is within the circle
			float distance = std::sqrt((i - x) * (i - x) + (j - y) * (j - y));
			if (distance <= radius && IsWithinBounds(i, j) && !IsEmpty(i, j))
			{
				RemoveElementAt(i, j);
			}
		}
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

inline glm::ivec2 Grid::ConvertScreenToGrid(const glm::ivec2& screenPos) const
{
	// Check if mouse position is within the bounding box of the grid
	if (screenPos.x < m_GridInfo.pos.x || screenPos.x >= m_GridInfo.pos.x + m_GridInfo.columns * m_GridInfo.cellSize ||
		screenPos.y < m_GridInfo.pos.y || screenPos.y >= m_GridInfo.pos.y + m_GridInfo.rows * m_GridInfo.cellSize)
	{
		return glm::ivec2(-1, -1); // Out of grid bounds
	}

	// Calculate grid coordinates if within bounds
	int gridX = (screenPos.x - m_GridInfo.pos.x) / m_GridInfo.cellSize;
	int gridY = (screenPos.y - m_GridInfo.pos.y) / m_GridInfo.cellSize;

	return glm::ivec2(gridY, gridX);
}

void Grid::MoveElement(int x, int y, int newX, int newY)
{
	m_Elements[newX][newY] = m_Elements[x][y];
	m_Elements[x][y] = EMPTY_CELL;
}

void Grid::SwapElements(int x, int y, int newX, int newY)
{
	std::swap(m_Elements[x][y], m_Elements[newX][newY]);
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