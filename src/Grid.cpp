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
	m_NumChunksX = (m_GridInfo.rows + m_ChunkSize - 1) / m_ChunkSize;
	m_NumChunksY = (m_GridInfo.columns + m_ChunkSize - 1) / m_ChunkSize;
	m_CurrentDirtyChunks = std::vector<std::vector<bool>>(m_NumChunksX, std::vector<bool>(m_NumChunksY, true));
	m_NextDirtyChunks = std::vector<std::vector<bool>>(m_NumChunksX, std::vector<bool>(m_NumChunksY, false));
	//m_DirtyChunks = std::vector<std::vector<bool>>(m_NumChunksX, std::vector<bool>(m_NumChunksY, false));

}

Grid::~Grid()
{
}

void Grid::Init()
{
	m_SelectedElement = "Sand";
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

void Grid::Render(Window* window)
{
	RenderElements(window);
	RenderGrid(window);
	RenderBrush(window);

	ImGui::Begin("Element Selector");
	ImGui::SetWindowPos("Element Selector", { 1050, 20 });

	if (ImGui::BeginListBox("##Elements", ImVec2(200, 200)))
	{
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
	ImGui::Begin("Create New Element", nullptr, ImGuiWindowFlags_None);
	ImGui::SetWindowPos("Create New Element", { 1260, 20 });

	static int elementCount = 1;

	static char elementName[64] = "";
	if (strlen(elementName) == 0) // If the name field is empty
	{
		snprintf(elementName, sizeof(elementName), "NewElement%d", elementCount);
	}

	static ImVec4 elementColor = { 1, 1, 1, 1 }; // Default color (white)
	static int mainComponent = -1;              // -1 = None, 0 = Solid, 1 = Liquid, 2 = Gas
	static bool hasGravity = true;             // Gravity component (extra for Solid/Liquid)

	// Input for element name
	ImGui::InputText("Name", elementName, IM_ARRAYSIZE(elementName));

	// Color picker
	ImGui::ColorEdit3("Color", (float*)&elementColor);

	// Main comp specific inputs
	static float solidDensity{ 1.f };

	static float liquidDensity{ 0.1f };
	static float liquidDispersionRate{ 5.f };

	static float gasDensity{ 0.1f };

	static float gravityScale{ 2.f };


	// Spreading comp inputs
	static bool hasSpreading{};
	static float spreadFactor{ 0.3f };
	static float spreadChance{ 0.25f };

	// Spreadable comp inputs
	static bool hasSpreadable{};
	static float spreadThreshold{ 1.f };
	static int spreadResistance{ 4 };

	// Lifetime comp inputs
	static bool hasLifeTime{};
	static float minLifeTime{ 1.f };
	static float maxLifeTime{ 1.5f };
	static char spawnElementName[64]{ "" };


	// Main component selection (radio buttons)
	ImGui::Text("Main Component:");
	if (ImGui::RadioButton("None", &mainComponent, -1)) { hasGravity = false; } // Reset invalid extras
	if (ImGui::RadioButton("Solid", &mainComponent, 0)) { hasGravity = true; }
	if (ImGui::RadioButton("Liquid", &mainComponent, 1)) { hasGravity = true; }
	if (ImGui::RadioButton("Gas", &mainComponent, 2)) { hasGravity = false; }

	// Extra components
	ImGui::Separator();
	if (mainComponent == 0) // Solid
	{
		ImGui::Text("Solid Component Settings");
		ImGui::InputFloat("Density (Solid)", &solidDensity, 0.1f, 1.0f, "%.2f");
		ImGui::Checkbox("Gravity", &hasGravity);
		if (hasGravity)
		{
			ImGui::InputFloat("Gravity Scale", &gravityScale, 0.1f, 1.0f, "%.2f");
		}
	}
	else if (mainComponent == 1) // Liquid
	{
		ImGui::Text("Liquid Component Settings");
		ImGui::InputFloat("Density (Liquid)", &liquidDensity, 0.1f, 1.0f, "%.2f");
		ImGui::InputFloat("Dispersion Rate", &liquidDispersionRate, 1.0f, 10.0f, "%.2f");
		ImGui::Checkbox("Gravity", &hasGravity);
		if (hasGravity)
		{
			ImGui::InputFloat("Gravity Scale", &gravityScale, 0.1f, 1.0f, "%.2f");
		}
	}
	else if (mainComponent == 2) // Gas
	{
		ImGui::Text("Gas Component Settings");
		ImGui::InputFloat("Density (Gas)", &gasDensity, 0.1f, 1.0f, "%.2f");
	}
	else if (mainComponent == -1) // None
	{
		ImGui::Text("No main component selected.");
	}

	// Additional Components
	ImGui::Separator();
	ImGui::Text("Optional Components:");

	// Spreadable Component
	ImGui::Checkbox("Spreadable Component", &hasSpreadable);
	if (hasSpreadable)
	{
		ImGui::Text("Spreadable Component Settings");
		ImGui::InputFloat("Spread Threshold", &spreadThreshold, 0.1f, 1.0f, "%.2f");
		ImGui::InputInt("Spread Resistance", &spreadResistance, 0.1, 1.0);
	}

	// Spreading Component
	ImGui::Checkbox("Spreading Component", &hasSpreading);
	if (hasSpreading)
	{
		ImGui::Text("Spreading Component Settings");
		ImGui::InputFloat("Spread Factor", &spreadFactor, 0.1f, 1.0f, "%.2f");
		ImGui::InputFloat("Spread Chance", &spreadChance, 0.01f, 0.1f, "%.2f");
	}

	// LifeTime Component
	ImGui::Checkbox("Life Time Component", &hasLifeTime);
	if (hasLifeTime)
	{
		ImGui::Text("LifeTime Component Settings");
		ImGui::InputFloat("Min Life Time", &minLifeTime, 1.0f, 10.0f, "%.2f");
		ImGui::InputFloat("Max Life Time", &maxLifeTime, 1.0f, 10.0f, "%.2f");
		ImGui::InputText("Element to Spawn", spawnElementName, IM_ARRAYSIZE(spawnElementName));
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
			components["Solid"] = SolidComp{ solidDensity };
			break;
		case 1: // Liquid
			components["Liquid"] = LiquidComp{ liquidDensity, liquidDispersionRate };
			break;
		case 2: // Gas
			components["Gas"] = GasComp{ gasDensity};
			break;
		case -1: // None
			// Leave components empty
			break;
		}

		// Add extra components
		if (mainComponent != 2 && hasGravity) // Gas and None cannot have Gravity
			components["Gravity"] = GravityComp{ gravityScale };

		// Add optional components
		if (hasSpreadable)
			components["Spreadable"] = SpreadableComp{ spreadThreshold, spreadResistance };
		if (hasSpreading)
			components["Spreading"] = SpreadingComp{ spreadFactor, spreadChance };
		if (hasLifeTime)
			components["Lifetime"] = LifeTimeComp{ minLifeTime, maxLifeTime, spawnElementName };

		// Add the new element to the registry
		m_pElementRegistry->AddElementType({ elementName, hexColor, components });

		// Increment element count and reset inputs
		elementCount++;
		snprintf(elementName, sizeof(elementName), "NewElement%d", elementCount); // Reset name field
		elementColor = { 1, 1, 1, 1 };
		mainComponent = -1; // Default to None
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

void Grid::RenderGrid(Window* window)
{
	static bool m_ShowDirtyChunks{};
	static bool m_ShowChunks{};

	ImGui::Begin("Debug");

	ImGui::SetWindowPos("Debug", { 20, 625 });

	if (ImGui::Button("Clear Grid"))
	{
		ClearGrid();
	}

	ImGui::Checkbox("Show Chunks", &m_ShowChunks);
	ImGui::Checkbox("Show Dirty Chunks", &m_ShowDirtyChunks);
	ImGui::Checkbox("Brush Overriding", &m_BrushOverride);

	ImGui::End();

	if (m_ShowChunks)
	{
		for (int chunkX = 0; chunkX < m_NumChunksX; ++chunkX)
		{
			for (int chunkY = 0; chunkY < m_NumChunksY; ++chunkY)
			{
				SDL_SetRenderDrawColor(window->GetSDLRenderer(), 0, 150, 0, 255);
				int startX = m_GridInfo.pos.x + chunkX * m_ChunkSize * m_GridInfo.cellSize;
				int startY = m_GridInfo.pos.y + chunkY * m_ChunkSize * m_GridInfo.cellSize;
				int width = m_ChunkSize * m_GridInfo.cellSize;
				int height = m_ChunkSize * m_GridInfo.cellSize;

				SDL_Rect chunkBounds = { startY, startX, width, height };
				SDL_RenderDrawRect(window->GetSDLRenderer(), &chunkBounds);
			}
		}
	}

	if (m_ShowDirtyChunks)
	{
		for (int chunkX = 0; chunkX < m_NumChunksX; ++chunkX)
		{
			for (int chunkY = 0; chunkY < m_NumChunksY; ++chunkY)
			{
				if (m_CurrentDirtyChunks[chunkX][chunkY])
				{
					SDL_SetRenderDrawColor(window->GetSDLRenderer(), 150, 150, 0, 255);
					int startX = m_GridInfo.pos.x + chunkX * m_ChunkSize * m_GridInfo.cellSize;
					int startY = m_GridInfo.pos.y + chunkY * m_ChunkSize * m_GridInfo.cellSize;
					int width = m_ChunkSize * m_GridInfo.cellSize;
					int height = m_ChunkSize * m_GridInfo.cellSize;

					SDL_Rect chunkBounds = { startY, startX, width, height };
					SDL_RenderDrawRect(window->GetSDLRenderer(), &chunkBounds);
				}
			}
		}
	}

	// Draw grid
	SDL_SetRenderDrawColor(window->GetSDLRenderer(), 8, 8, 8, 255);
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

	// Create texture once if it doesn't exist
	if (!gridTexture)
	{
		gridTexture = SDL_CreateTexture(
			window->GetSDLRenderer(),
			SDL_PIXELFORMAT_RGB888,
			SDL_TEXTUREACCESS_STREAMING,
			this->GetColumns(),
			this->GetRows()
		);
	}

	// Check if any chunk is dirty
	bool hasDirtyChunks = false;
	for (int chunkX = 0; chunkX < m_NumChunksX; ++chunkX)
	{
		for (int chunkY = 0; chunkY < m_NumChunksY; ++chunkY)
		{
			if (m_CurrentDirtyChunks[chunkX][chunkY])
			{
				hasDirtyChunks = true;
				break;
			}
		}
		if (hasDirtyChunks) break;
	}

	// Update the texture only if there are dirty chunks
	if (hasDirtyChunks)
	{
		void* pixels;
		int pitch;
		SDL_LockTexture(gridTexture, nullptr, &pixels, &pitch);

		Uint32* pixelData = static_cast<Uint32*>(pixels);
		const int PIXELS_PER_ROW = pitch / 4; // Uint32 (4 bytes per pixel)

		// Update all pixels in the grid
		for (int x = 0; x < this->GetRows(); ++x)
		{
			for (int y = 0; y < this->GetColumns(); ++y)
			{
				if (!IsEmpty(x, y))
				{
					const Element* element = GetElementData(x, y);
					uint32_t baseColor = element->definition->color;

					// Apply element's tint to color
					uint8_t r = (baseColor >> 16) & 0xFF;
					uint8_t g = (baseColor >> 8) & 0xFF;
					uint8_t b = baseColor & 0xFF;

					auto adjustColor = [tint = element->tint](uint8_t channel) -> uint8_t {
						int newChannel = std::clamp(static_cast<int>(channel) + tint, 0, 255);
						return static_cast<uint8_t>(newChannel);
						};

					r = adjustColor(r);
					g = adjustColor(g);
					b = adjustColor(b);

					uint32_t color = (r << 16) | (g << 8) | b;

					// Update pixel data
					pixelData[x * PIXELS_PER_ROW + y] = color;
				}
				else
				{
					// Set empty cells to the background color
					pixelData[x * PIXELS_PER_ROW + y] = 0x1A1A1A; // Black with full opacity
				}
			}
		}

		SDL_UnlockTexture(gridTexture);
	}

	// Render the texture to the screen (always)
	SDL_Rect destRect = {
		m_GridInfo.pos.x,
		m_GridInfo.pos.y,
		m_GridInfo.cellSize * this->GetColumns(),
		m_GridInfo.cellSize * this->GetRows()
	};

	SDL_RenderCopy(window->GetSDLRenderer(), gridTexture, nullptr, &destRect);
}

bool Grid::IsChunkDirty(int chunkX, int chunkY)
{
	if (chunkX >= 0 && chunkX < m_NumChunksX && chunkY >= 0 && chunkY < m_NumChunksY)
	{
		return m_CurrentDirtyChunks[chunkX][chunkY];
	}
	return false;
}

void Grid::MarkChunkAsDirty(int x, int y)
{
	int chunkX = x / m_ChunkSize;
	int chunkY = y / m_ChunkSize;

	// Mark the main chunk dirty
	if (chunkX >= 0 && chunkX < m_NumChunksX && chunkY >= 0 && chunkY < m_NumChunksY)
	{
		m_NextDirtyChunks[chunkX][chunkY] = true;
	}

	// Check and mark adjacent chunks
	// Check if on the right border
	if (x % m_ChunkSize == m_ChunkSize - 1 && chunkX + 1 < m_NumChunksX)
	{
		m_NextDirtyChunks[chunkX + 1][chunkY] = true;
	}

	// Check if on the left border
	if (x % m_ChunkSize == 0 && chunkX - 1 >= 0)
	{
		m_NextDirtyChunks[chunkX - 1][chunkY] = true;
	}

	// Check if on the bottom border
	if (y % m_ChunkSize == m_ChunkSize - 1 && chunkY + 1 < m_NumChunksY)
	{
		m_NextDirtyChunks[chunkX][chunkY + 1] = true;
	}

	// Check if on the top border
	if (y % m_ChunkSize == 0 && chunkY - 1 >= 0)
	{
		m_NextDirtyChunks[chunkX][chunkY - 1] = true;
	}
}


void Grid::UnmarkChunkAsDirty(int x, int y)
{
	int chunkX = x / m_ChunkSize;
	int chunkY = y / m_ChunkSize;

	if (chunkX >= 0 && chunkX < m_NumChunksX && chunkY >= 0 && chunkY < m_NumChunksY)
	{
		m_NextDirtyChunks[chunkX][chunkY] = false;
	}
}

void Grid::ResetDirtyChunks()
{
	for (auto& row : m_NextDirtyChunks)
	{
		std::fill(row.begin(), row.end(), false);
	}
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
	if (id == EMPTY_CELL) return nullptr;
	return m_pElementRegistry->GetElementData(id);
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
		MarkChunkAsDirty(x, y);

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
		MarkChunkAsDirty(x, y);


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
	if (x == newX && y == newY) return;

	MarkChunkAsDirty(x, y);
	MarkChunkAsDirty(newX, newY);

	m_Elements[newX][newY] = m_Elements[x][y];
	m_Elements[x][y] = EMPTY_CELL;
}

void Grid::SwapElements(int x, int y, int newX, int newY)
{
	if (x == newX && y == newY) return;

	MarkChunkAsDirty(x, y);
	MarkChunkAsDirty(newX, newY);

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