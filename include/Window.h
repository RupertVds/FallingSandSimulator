#ifndef WINDOW_H
#define WINDOW_H

#include <SDL.h>
#include <iostream>

class Window {
public:
    Window(const std::string& title, int width, int height);
    ~Window();
    bool Init();
    void Clear() const;
    void Update() const;
    SDL_Renderer* GetSDLRenderer() const { return m_pRenderer; }
    SDL_Window* GetSDLWindow() const { return m_pWindow; }
    int GetHeight() const { return m_Height; }
    int GetWidth() const { return m_Width; }

private:
    std::string m_Title;
    int m_Width;
    int m_Height;
    SDL_Window* m_pWindow;
    SDL_Renderer* m_pRenderer; // SDL Renderer
};

#endif // WINDOW_H
