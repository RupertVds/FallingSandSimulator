#ifndef GAME_H
#define GAME_H

#include "Window.h"

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
    void Update();
    void Render() const;
    void ProcessInput();  // Handles all input, including window close

private:
    Window* m_window;
    bool m_isRunning;
    int m_BrushSize;
};

#endif // !GAME_H
