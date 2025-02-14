#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <glm/glm.hpp>
#include <unordered_map>
#include <SDL.h>

class InputManager final
{
public:
    static InputManager& GetInstance()
    {
        static InputManager instance;
        return instance;
    }

    InputManager(const InputManager& other) = delete;
    InputManager& operator=(const InputManager& other) = delete;
    InputManager(InputManager&& other) = delete;
    InputManager& operator=(InputManager&& other) = delete;
    ~InputManager() = default;
public:
    void Update();
    void HandleSDLEvent(const SDL_Event& event);

    const glm::vec2& GetMousePos() const { return m_MousePos; };
    // Mouse button state checks
    bool IsMouseButtonPressed(int button) const;
    bool IsMouseButtonHeld(int button) const;
    bool IsMouseButtonReleased(int button) const;

    // Keyboard key state checks
    bool IsKeyPressed(SDL_Scancode key) const;
    bool IsKeyHeld(SDL_Scancode key) const;
    bool IsKeyReleased(SDL_Scancode key) const;

    // Scroll state checks
    bool IsScrolledUp() const { return m_ScrolledUp; }
    bool IsScrolledDown() const { return m_ScrolledDown; }
    void ResetScrollState();
private:
    glm::vec2 m_MousePos{};
    std::unordered_map<int, bool> m_MouseButtonStateCurrent{};
    std::unordered_map<int, bool> m_MouseButtonStatePrevious{};

    std::unordered_map<int, bool> m_KeyPressedMap{};
    std::unordered_map<int, bool> m_KeyHeldMap{};
    std::unordered_map<int, bool> m_KeyReleasedMap{};

    // Scroll state
    bool m_ScrolledUp{ false };
    bool m_ScrolledDown{ false };

    InputManager() = default;

    void UpdateMousePosition();
    void UpdateMouseButtons();
    void UpdateKeyboardState();
};

#endif // !INPUTMANAGER_H

