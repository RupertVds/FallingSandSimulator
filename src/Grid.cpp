#include "Grid.h"
#include <SDL2/SDL.h>
#include <thread>
#include "InputManager.h"
#include "ServiceLocator.h"
#include <Systems.h>
#include "Utils.h"
#include <algorithm>
#include <imgui.h>
#include <unordered_map>

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
	m_SelectedElement = "Sand";
	m_PreviousGridMousePos = ConvertScreenToGrid(InputManager::GetInstance().GetMousePos());

	//for (int x = 0; x < this->GetRows(); ++x)
	//{
	//	for (int y = 0; y < this->GetColumns(); ++y)
	//	{
	//		AddElementAt(x, y, "Water");
	//	}
	//}
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
		m_SelectedElement = element->definition->name;
	}
	// CLICK TO PLACE IT
	else if (InputManager::GetInstance().IsKeyHeld(SDL_SCANCODE_LSHIFT) && InputManager::GetInstance().IsMouseButtonHeld(SDL_BUTTON_LEFT))
	{
		if (m_PreviousGridMousePos == glm::ivec2{ -1, -1 })
		{
			m_PreviousGridMousePos = gridMousePos;
		}

		BresenhamLine(m_PreviousGridMousePos, gridMousePos, [&](int x, int y)
			{
				AddElementBrushed(x, y, m_SelectedElement, m_BrushOverride, 1.f);
				return true;
			}
		);
	}
	else if (InputManager::GetInstance().IsMouseButtonHeld(SDL_BUTTON_LEFT))
	{
		if (m_PreviousGridMousePos == glm::ivec2{ -1, -1 })
		{
			m_PreviousGridMousePos = gridMousePos;
		}

		BresenhamLine(m_PreviousGridMousePos, gridMousePos, [&](int x, int y)
			{
				AddElementBrushed(x, y, m_SelectedElement, m_BrushOverride, 0.01f);
				return true;
			}
		);
	}

	if (InputManager::GetInstance().IsMouseButtonHeld(SDL_BUTTON_RIGHT))
	{
		if (m_PreviousGridMousePos == glm::ivec2{ -1, -1 })
		{
			m_PreviousGridMousePos = gridMousePos;
		}

		BresenhamLine(m_PreviousGridMousePos, gridMousePos, [&](int x, int y)
			{
				RemoveElementBrushed(x, y);
				return true;
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
				AddElementBrushed(x, y, m_SelectedElement, m_BrushOverride, 1.f);
				return true;
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
	
	if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_1))
	{
		m_SelectedElement = "Wall";
	}
	if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_2))
	{
		m_SelectedElement = "Sand";
	}
	if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_3))
	{
		m_SelectedElement = "Water";
	}
	if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_4))
	{
		m_SelectedElement = "Smoke";
	}
	if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_5))
	{
		m_SelectedElement = "Wood";
	}
	if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_6))
	{
		m_SelectedElement = "Fire";
	}

	if (InputManager::GetInstance().IsKeyPressed(SDL_SCANCODE_DELETE))
	{
		m_BrushOverride = !m_BrushOverride;
		if (m_BrushOverride)
		{
			std::cout << "Brush Overriding is ON!\n";
		}
		else
		{
			std::cout << "Brush Overriding is OFF!\n";
		}
	}

	m_PreviousGridMousePos = gridMousePos;
}

void Grid::FixedUpdate()
{
	UpdateElements();

	++m_FrameCounter;
}

void Grid::UpdateElements()
{
	UpdateGridElements(*this);
}

void Grid::Render(Window* window) const
{
	RenderElements(window);
	RenderGrid(window);
	RenderBrush(window);

	ImGui::NewFrame();
	ImGui::Begin("Element Selector");

	if (ImGui::BeginListBox("##Elements", ImVec2(300, 200)))
	{ // Adjust size as needed
		const auto& elementTypes = m_pElementRegistry->GetElementTypes();

		for (const auto& [name, definition] : elementTypes)
		{
			ImGui::PushID(name.c_str());

			// Convert hex color to ImVec4
			ImVec4 color = HexToImVec4(definition.color);
			ImGui::ColorButton("Color", color, 0, ImVec2(20, 20));
			ImGui::SameLine();
			if (ImGui::Selectable(name.c_str(), m_SelectedElement == name)) 
			{
				m_SelectedElement = name;
			}

			ImGui::PopID();
		}
		ImGui::EndListBox();
	}

	ImGui::End();

	// New Element Creator
	ImGui::Begin("Create New Element");

	static int elementCount = 1;

	static char elementName[64] = "";
	if (strlen(elementName) == 0) // If the name field is empty
	{
		snprintf(elementName, sizeof(elementName), "NewElement%d", elementCount);
	}	
	
	static ImVec4 elementColor = { 1, 1, 1, 1 }; // Default color (white)
	static int mainComponent = 0;          // 0 = Solid, 1 = Liquid, 2 = Gas
	static bool hasGravity = false;        // Gravity component (extra for Solid/Liquid)

	// Input for element name
	ImGui::InputText("Name", elementName, IM_ARRAYSIZE(elementName));

	// Color picker
	ImGui::ColorEdit3("Color", (float*)&elementColor);

	// Main component selection (radio buttons)
	ImGui::Text("Main Component:");
	if (ImGui::RadioButton("Solid", &mainComponent, 0)) { hasGravity = false; } // Reset invalid extras
	if (ImGui::RadioButton("Liquid", &mainComponent, 1)) { hasGravity = false; }
	if (ImGui::RadioButton("Gas", &mainComponent, 2)) { hasGravity = false; }

	// Extra components
	ImGui::Text("Extra Components:");
	if (mainComponent == 0) // Solid
	{
		ImGui::Checkbox("Gravity", &hasGravity); // Gravity allowed for Solid
	}
	else if (mainComponent == 1) // Liquid
	{
		ImGui::Checkbox("Gravity", &hasGravity); // Gravity allowed for Liquid
	}
	else if (mainComponent == 2) // Gas
	{
		ImGui::Text("No Gravity component allowed for Gas");
	}

	// Add Element button
	if (ImGui::Button("Add Element"))
	{
		// Convert ImVec4 to hex color
		uint32_t hexColor = ((int)(elementColor.x * 255) << 16) |
			((int)(elementColor.y * 255) << 8) |
			((int)(elementColor.z * 255));

		// Build the component list
		std::unordered_map<std::string, Component> components;

		// Add the selected main component
		switch (mainComponent)
		{
		case 0: // Solid
			components["Solid"] = SolidComp{ 1.0f }; // Example density
			break;
		case 1: // Liquid
			components["Liquid"] = LiquidComp{ 0.1f, 15.0f }; // Example values
			break;
		case 2: // Gas
			components["Gas"] = GasComp{ 0.1f, 15.0f }; // Example values
			break;
		}

		// Add extra components
		if (mainComponent != 2 && hasGravity) // Gas cannot have Gravity
			components["Gravity"] = GravityComp{ 2.0f };

		// Add the new element to the registry
		m_pElementRegistry->AddElementType({ elementName, hexColor, components });

		// Increment element count and reset inputs
		elementCount++;
		snprintf(elementName, sizeof(elementName), "NewElement%d", elementCount); // Reset name field
		elementColor = { 1, 1, 1, 1 };
		mainComponent = 0; // Default to Solid
		hasGravity = false;
	}

	ImGui::End();
	ImGui::Render();

}

void Grid::RenderBrush(Window* window) const
{
	if (!m_MouseIsInGrid) return;

	// Get mouse position in grid space
	const glm::ivec2 gridMousePos = ConvertScreenToGrid(InputManager::GetInstance().GetMousePos());

	// Define grid properties
	int gridOriginX = m_GridInfo.pos.x; // X position of grid origin on screen
	int gridOriginY = m_GridInfo.pos.y; // Y position of grid origin on screen
	int cellSize = m_GridInfo.cellSize; // Size of a single cell in pixels

	// Calculate the center of the brush in screen space
	int screenCenterX = gridOriginX + gridMousePos.x * cellSize + cellSize / 2;
	int screenCenterY = gridOriginY + gridMousePos.y * cellSize + cellSize / 2;

	// Circle properties
	float radius = m_BrushSize * cellSize; // Radius in pixels
	int segments = 100;                    // Number of segments to approximate the circle

	// Set the draw color
	SDL_SetRenderDrawColor(window->GetSDLRenderer(), 255, 255, 255, 128);

	// Calculate and draw the circle using line segments
	for (int i = 0; i < segments; ++i)
	{
		float theta1 = (2.0f * M_PI * i) / segments;
		float theta2 = (2.0f * M_PI * (i + 1)) / segments;

		int x1 = static_cast<int>(screenCenterX + radius * cos(theta1));
		int y1 = static_cast<int>(screenCenterY + radius * sin(theta1));

		int x2 = static_cast<int>(screenCenterX + radius * cos(theta2));
		int y2 = static_cast<int>(screenCenterY + radius * sin(theta2));

		SDL_RenderDrawLine(window->GetSDLRenderer(), y1, x1, y2, x2);
	}
}

void Grid::RenderGrid(Window* window) const
{
	SDL_SetRenderDrawColor(window->GetSDLRenderer(), 255, 0, 0, 255); // Bright red for bounds
	SDL_Rect gridBounds = {
		m_GridInfo.pos.x,
		m_GridInfo.pos.y,
		m_GridInfo.columns * m_GridInfo.cellSize,
		m_GridInfo.rows * m_GridInfo.cellSize
	};
	SDL_RenderDrawRect(window->GetSDLRenderer(), &gridBounds);
}

void Grid::RenderElements(Window* window) const
{
	static SDL_Texture* gridTexture = nullptr;
	//static bool textureNeedsUpdate = true;

	// create texture if no texture
	if (!gridTexture)
	{
		gridTexture = SDL_CreateTexture(
			window->GetSDLRenderer(),
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
					uint32_t baseColor = element->definition->color;

					// apply element's tint to color
					// Extract RGB channels
					uint8_t r = (baseColor >> 16) & 0xFF;
					uint8_t g = (baseColor >> 8) & 0xFF;
					uint8_t b = baseColor & 0xFF;

					// Adjust channels based on tint
					auto adjustColor = [tint = element->tint](uint8_t channel) -> uint8_t {
						int newChannel = std::clamp(static_cast<int>(channel) + tint, 0, 255);
						return static_cast<uint8_t>(newChannel);
						};

					r = adjustColor(r);
					g = adjustColor(g);
					b = adjustColor(b);

					// Combine adjusted channels into a single color
					uint32_t color = (r << 16) | (g << 8) | b;


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

	SDL_RenderCopy(window->GetSDLRenderer(), gridTexture, nullptr, &destRect);
}

inline ElementID Grid::GetElementID(int x, int y) const
{
	return m_Elements[x][y];
}

ElementID Grid::GetElementID(const glm::ivec2& pos) const
{
	return GetElementID(pos.x, pos.y);
}

inline Element* Grid::GetElementData(int x, int y) const
{
	ElementID id = GetElementID(x, y);
	return id != EMPTY_CELL ? m_pElementRegistry->GetElementData(id) : nullptr;
}

Element* Grid::GetElementData(const glm::ivec2& pos) const
{
	return GetElementData(pos.x, pos.y);
}

inline bool Grid::IsWithinBounds(int x, int y) const
{
	return x >= 0 && x < m_GridInfo.rows && y >= 0 && y < m_GridInfo.columns;
}

inline bool Grid::IsWithinBounds(const glm::ivec2& pos) const
{
	return IsWithinBounds(pos.x , pos.y);
}

inline bool Grid::IsEmpty(int x, int y) const
{
	return GetElementID(x, y) == EMPTY_CELL;
}

inline bool Grid::IsEmpty(const glm::ivec2& pos) const
{
	return IsEmpty(pos.x, pos.y);
}

inline bool Grid::IsEvenFrame() const
{
	return m_FrameCounter % 2 == 0;
}

void Grid::AddElementBrushed(int x, int y, const std::string& elementTypeName, bool override, float spawnChance)
{
	float radius = m_BrushSize - 0.5f; // Fractional brush size

	// Clamp spawnChance between 0 and 1 for safety
	spawnChance = std::clamp(spawnChance, 0.0f, 1.0f);

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
			if (distance <= radius && IsWithinBounds(i, j) && (override ? true : IsEmpty(i, j)))
			{
				// Roll a random chance to skip adding an element
				if (static_cast<float>(rand()) / RAND_MAX > spawnChance)
				{
					continue; // Skip this cell
				}

				// Handle element addition/removal
				if (override) RemoveElementAt(i, j);
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