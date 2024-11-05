#ifndef GRID_H
#define GRID_H

#include "Cell.h"
#include <vector>
#include "Window.h"

struct GridInfo
{
	glm::ivec2 pos{};
	int rows{};
	int columns{};
	int cellSize{};
};

class Grid final
{
public:
	Grid(const GridInfo& gridInfo);
	~Grid();

	Grid(const Grid& other) = delete;
	Grid& operator=(const Grid& other) = delete;
	Grid(Grid&& other) = delete;
	Grid& operator=(Grid&& other) = delete;
public:
	int GetColumns() const { return m_GridInfo.columns; };
	int GetRows() const { return m_GridInfo.rows; };

	inline Cell& GetCell(int x, int y)
	{
		if (x >= m_GridInfo.rows) std::cout << "Trying to access row index bigger than existing amount of rows!!!\n";
		if (y >= m_GridInfo.columns) std::cout << "Trying to access row index bigger than existing amount of rows!!!\n";
		return m_Cells[x][y];
	}

	inline const std::vector<glm::ivec2>& GetSelectedCells() const
	{
		return m_SelectedCells;
	}

	inline bool IsWithinBounds(int x, int y) const
	{
		return x >= 0 && x < m_GridInfo.rows && y >= 0 && y < m_GridInfo.columns;
	}

	inline glm::ivec2 ConvertScreenToGrid(const glm::ivec2& screenPos) const
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

		return glm::ivec2(gridX, gridY);
	}

	void MoveElement(int x1, int y1, int x2, int y2)
	{
		std::swap(m_Cells[x1][y1].m_pElement, m_Cells[x2][y2].m_pElement);
	}

	void Update();
	void FixedUpdate();
	void Render(Window* window) const;

	void UpdateElements();
	void UpdateSelection();

	void RenderSelection(Window* window) const;
	void RenderGrid(Window* window) const;
	void RenderElements(Window* window) const;

	void ClearGrid();
private:
	// sorted so memory layout is optimal

	std::vector<std::vector<Cell>> m_Cells;
	GridInfo m_GridInfo{};

	// Selection Settings
	int m_SelectionBrushSize{1};
	glm::ivec2 m_SelectedCell{};
	std::vector<glm::ivec2> m_SelectedCells{};
	glm::vec4 m_SelectionColor{ 0.f, 100.f, 0.f, 255.f };
	// selection is dirty
	bool m_MouseIsInGrid{};
	bool m_SelectionIsDirty{};
};

#endif // !GRID_H