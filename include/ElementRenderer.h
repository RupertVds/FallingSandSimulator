#ifndef ELEMENTRENDERER_H
#define ELEMENTRENDERER_H

#include <vector>
#include <SDL_surface.h>
#include <unordered_map>

class ElementRenderer final
{
public:
	struct ColorRGB
	{
		float r{}, g{}, b{};
	};
	struct ElementRenderProperties
	{
		std::vector<ColorRGB> colors{};
	};
public:
	ElementRenderer()
	{
		//m_ElementRenderProperties[Element::ElementType::Sand] = { 
		//	{ {227, 186, 102}, {255, 255, 255} }
		//};

	}
	~ElementRenderer() = default;
	ElementRenderer(const ElementRenderer& other) = delete;
	ElementRenderer& operator=(const ElementRenderer& other) = delete;
	ElementRenderer(ElementRenderer&& other) = delete;
	ElementRenderer& operator=(ElementRenderer&& other) = delete;
public:
	//void RenderElementSDL(ElementType elementType, SDL_Surface* surface) const
	//{
	//	Uint32* pixels = static_cast<Uint32*>(surface->pixels);
	//	int pitch = surface->pitch / 4;  // pitch is in bytes, so divide by 4 to get pixel width

	//	//ColorRGB colorToMap{ 
	//	//	m_ElementRenderProperties.at(elementType).colors.begin()->r,
	//	//	m_ElementRenderProperties.at(elementType).colors.begin()->g,
	//	//	m_ElementRenderProperties.at(elementType).colors.begin()->b };

	//	//m_ElementRenderProperties.at(elementType).colors
	//	//colorToMap = 


	//	//Uint32 color = SDL_MapRGB(surface->format, colorToMap.r, colorToMap.g, colorToMap.b);

	//	// Set pixel color at position (x, y) in the SDL surface
	//	//pixels[y * pitch + x] = color;
	//}
private:
	//std::unordered_map<Element::ElementType, ElementRenderProperties> m_ElementRenderProperties{};
};

#endif // !ELEMENTRENDERER_H