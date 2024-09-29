#include "ServiceLocator.h"

std::unique_ptr<ISandSimulation> ServiceLocator::m_SandSimulationInstance = nullptr;

ISandSimulation& ServiceLocator::GetSandSimulator()
{
	return *m_SandSimulationInstance;
}

void ServiceLocator::RegisterSandSimulation(std::unique_ptr<ISandSimulation>&& ss)
{
	//m_SandSimulationInstance = ss == nullptr ? m_SandSimulationInstance = std::make_unique<ISandSimulation>() : std::move(ss);
	m_SandSimulationInstance = std::move(ss);
}
