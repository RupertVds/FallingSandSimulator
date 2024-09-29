#ifndef CPUSANDSIMULATION_H
#define CPUSANDSIMULATION_H

#include "ISandSimulation.h"
#include <vector>
#include <random>

class CPUSandSimulation final : public ISandSimulation
{
public:
	CPUSandSimulation(size_t width, size_t height);
	~CPUSandSimulation() override = default;

	void Update() override;
	void Render(SDL_Surface* surface) const override;
	void PlaceParticle(size_t x, size_t y) override;
	void ProcessSandParticle(int x, int y, std::mt19937& gen, std::uniform_int_distribution<>& dist);
private:
	size_t m_Width{};
	size_t m_Height{};

	std::vector<std::vector<int>> m_Grid{};
	std::random_device m_rd;
};

#endif // !CPUSANDSIMULATION_H