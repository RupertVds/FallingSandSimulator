#ifndef SERVICELOCATOR_H
#define SERVICELOCATOR_H

#include "ISandSimulation.h"
#include <memory>

class ServiceLocator final
{
public:
	static ISandSimulation& GetSandSimulator();
	static void RegisterSandSimulation(std::unique_ptr<ISandSimulation>&& ss);
private:
	static std::unique_ptr<ISandSimulation> m_SandSimulationInstance;
};

#endif // !SERVICELOCATOR_H