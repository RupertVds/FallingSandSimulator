#ifndef ISANDSIMULATION_H
#define ISANDSIMULATION_H

#include <SDL_surface.h>

class ISandSimulation
{
public:
	virtual ~ISandSimulation() = default;

	virtual void Update() = 0;
	virtual void Render(SDL_Surface* surface) const = 0;
	virtual void PlaceParticle(size_t x, size_t y) = 0;

};



#endif // !ISANDSIMULATION_H

