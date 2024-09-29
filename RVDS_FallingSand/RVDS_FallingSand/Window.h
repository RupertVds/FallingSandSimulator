#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>
#include <string>
#include <glad\glad.h>

class Window
{
public:
    Window(const std::string& title, int width, int height);
    ~Window();

    bool Init();
    void Clear() const;
    void Update() const;

    SDL_Window* GetWindow() const { return m_window; }
    SDL_Surface* GetSurface() const { return m_surface; }
    int GetWidth() const { return m_width; };
    int GetHeight() const { return m_height; };

private:
    std::string m_title;
    int m_width;
    int m_height;

    SDL_Window* m_window;
    SDL_Surface* m_surface;


};

#endif // WINDOW_H
