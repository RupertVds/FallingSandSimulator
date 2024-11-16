#ifndef GRID_H
#define GRID_H

#include <vector>
#include "Window.h"
#include <glm/glm.hpp>
#include "ElementRegistry.h"

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
	void Init();

	void Update();
	void FixedUpdate();
	void Render(Window* window) const;

	void UpdateElements();
	void UpdateInput();

	void RenderGrid(Window* window) const;
	void RenderElements(Window* window) const;
	void RenderBrush(Window* window) const;

	void AddElementBrushed(int x, int y, const std::string& elementTypeName);
	void AddElementAt(int x, int y, const std::string& elementTypeName);
	void RemoveElementBrushed(int x, int y);
	void RemoveElementAt(int x, int y);
	inline glm::ivec2 ConvertScreenToGrid(const glm::ivec2& screenPos) const;

	int GetRows() const { return m_GridInfo.rows; };
	int GetColumns() const { return m_GridInfo.columns; };

	inline ElementID GetElementID(int x, int y) const;
	inline Element* GetElementData(int x, int y) const;
	inline bool IsWithinBounds(int x, int y) const;
	inline bool IsEmpty(int x, int y) const;
	inline bool IsEvenFrame() const;

	void MoveElement(int x, int y, int newX, int newY);
	void SwapElements(int x, int y, int newX, int newY);
	void ClearGrid();
private:
	GridInfo m_GridInfo{};

	std::vector<std::vector<ElementID>> m_Elements{};
	std::unique_ptr<ElementRegistry> m_pElementRegistry{};

	// Brush Settings
	int m_BrushSize{1};
	glm::vec3 m_BrushColor{ 0.f, 100.f, 0.f };

	glm::ivec2 m_PreviousGridMousePos{};
	std::string m_ElementToDraw{};
	bool m_MouseIsInGrid{};

	uint8_t m_FrameCounter{};
};

#endif // !GRID_H