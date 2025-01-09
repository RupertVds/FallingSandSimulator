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
	inline float GetFixedTimeStep() const override;
	void SetFixedTimeStep(float fixedTimeStep) override;
private:
	bool m_IsSimulating{ true };
	std::unique_ptr<Grid> m_pGrid{};
	Window* m_pWindow{};
	float m_FixedTimeStep{};

	void Init() override;
};

#endif // !CPUSANDSIMULATION_H