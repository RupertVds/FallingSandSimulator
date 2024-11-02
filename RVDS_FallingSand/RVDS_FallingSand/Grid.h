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
		return m_Cells[x][y];
	}

	inline bool IsWithinBounds(int x, int y)
	{
		return x >= 0 && y >= 0 && x < m_GridInfo.columns && y < m_GridInfo.rows;
	}

	void MoveElement(int x1, int y1, int x2, int y2)
	{
		std::swap(m_Cells[x1][y1].m_pElement, m_Cells[x2][y2].m_pElement);
	}

	void Update();
	void Render(Window* window) const;

private:
	GridInfo m_GridInfo{};

	std::vector<std::vector<Cell>> m_Cells;
};

#endif // !GRID_H