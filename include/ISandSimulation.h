#ifndef ISANDSIMULATION_H
#define ISANDSIMULATION_H

#include <SDL_surface.h>
#include <memory>

class ISandSimulation
{
public:
	virtual ~ISandSimulation() = default;

	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void FixedUpdate() = 0;
	virtual void Render() const = 0;
	virtual bool IsActive() const = 0;
	virtual float GetFixedTimeStep() const = 0;
	virtual void SetFixedTimeStep(float fixedTimeStep) = 0;
};

#endif // !ISANDSIMULATION_H