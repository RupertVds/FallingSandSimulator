#ifndef GAME_H
#define GAME_H

#include "Window.h"
#include <chrono>

class Game final
{
public:
    Game();
    ~Game();

    Game(const Game& other) = delete;
    Game& operator=(const Game& other) = delete;
    Game(Game&& other) noexcept = delete;
    Game& operator=(Game&& other) = delete;

    void Run();
    void Update(float deltaTime);
    void Render() const;
    void ProcessInput();  // Handles all input, including window close

private:
    Window* m_pWindow;
    bool m_IsRunning;
    int m_BrushSize;
    std::chrono::steady_clock::time_point m_LastTime;
};

#endif // !GAME_H
