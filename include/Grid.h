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
	void Render(Window* window);

	void UpdateElements();
	void UpdateInput();

	void RenderGrid(Window* window) const;
	void RenderElements(Window* window) const;
	void RenderBrush(Window* window) const;

	void AddElementBrushed(int x, int y, const std::string& elementTypeName, bool override, float spawnChance = 1.0f);
	void AddElementAt(int x, int y, const std::string& elementTypeName);
	void RemoveElementBrushed(int x, int y);
	void RemoveElementAt(int x, int y);
	inline glm::ivec2 ConvertScreenToGrid(const glm::ivec2& screenPos) const;

	int GetRows() const { return m_GridInfo.rows; };
	int GetColumns() const { return m_GridInfo.columns; };

	int GetNumChunksX() const { return m_NumChunksX; };
	int GetNumChunksY() const { return m_NumChunksY; };
	int GetChunkSize() const { return m_ChunkSize; };

	bool IsChunkDirty(int chunkX, int chunkY);
	void MarkChunkAsDirty(int x, int y);
	void UnmarkChunkAsDirty(int x, int y);
	void ResetDirtyChunks();

	inline ElementID GetElementID(int x, int y) const;
	inline ElementID GetElementID(const glm::ivec2& pos) const;
	inline Element* GetElementData(int x, int y) const;
	inline Element* GetElementData(const glm::ivec2& pos) const;
	inline bool IsWithinBounds(int x, int y) const;
	inline bool IsWithinBounds(const glm::ivec2& pos) const;
	inline bool IsEmpty(int x, int y) const;
	inline bool IsEmpty(const glm::ivec2& pos) const;
	inline bool IsEvenFrame() const;
	const ElementRegistry* GetElementRegistry() const { return m_pElementRegistry.get(); };

	void MoveElement(int x, int y, int newX, int newY);
	void SwapElements(int x, int y, int newX, int newY);
	void ClearGrid();
	mutable std::vector<std::vector<bool>> m_CurrentDirtyChunks;
	mutable std::vector<std::vector<bool>> m_NextDirtyChunks;
private:
	GridInfo m_GridInfo{};

	const int m_ChunkSize{ 32 };


	int m_NumChunksX{};
	int m_NumChunksY{};

	std::vector<std::vector<ElementID>> m_Elements{};
	std::unique_ptr<ElementRegistry> m_pElementRegistry{};

	// Brush Settings
	int m_BrushSize{ 4 };
	glm::vec3 m_BrushColor{ 0.f, 100.f, 0.f };
	bool m_BrushOverride{};
	const float m_SpawnBrushTime{ 1 };
	float m_SpawnBrushTimer{};


	glm::ivec2 m_PreviousGridMousePos{};
	mutable std::string m_SelectedElement{};
	bool m_MouseIsInGrid{};

	uint8_t m_FrameCounter{};
};

#endif // !GRID_H