#ifndef CPUSANDSIMULATION_H
#define CPUSANDSIMULATION_H

#include "ISandSimulation.h"
#include "Grid.h"
#include <vector>
#include <random>
#include <memory>

class CPUSandSimulation final : public ISandSimulation
{
public:

	CPUSandSimulation(const GridInfo& gridInfo, Window* window);
	~CPUSandSimulation() override = default;

	void Update() override;
	void FixedUpdate() override;
	void Render() const override;

	bool IsActive() const override;

	void PlaceParticle(size_t x, size_t y, std::unique_ptr<Element>&& element) override;
	void ProcessSandParticle(int x, int y, std::mt19937& gen, std::uniform_int_distribution<>& dist);
private:
	bool m_IsSimulating{ false };
	std::unique_ptr<Grid> m_pGrid{};
	Window* m_pWindow{};

	void Init() override;
};

#endif // !CPUSANDSIMULATION_H