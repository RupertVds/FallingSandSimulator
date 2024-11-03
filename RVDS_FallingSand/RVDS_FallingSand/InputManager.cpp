#include "InputManager.h"

void InputManager::Update()
{
    UpdateMousePosition();
    UpdateMouseButtons();
    UpdateKeyboardState();
}

void InputManager::UpdateMousePosition()
{
    int x, y;
    SDL_GetMouseState(&x, &y);
    m_MousePos = { static_cast<float>(x), static_cast<float>(y) };
}

void InputManager::UpdateMouseButtons()
{
    m_MouseButtonStatePrevious = m_MouseButtonStateCurrent;

    Uint32 mouseButtons = SDL_GetMouseState(nullptr, nullptr);
    m_MouseButtonStateCurrent[SDL_BUTTON_LEFT] = mouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT);
    m_MouseButtonStateCurrent[SDL_BUTTON_RIGHT] = mouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT);
    m_MouseButtonStateCurrent[SDL_BUTTON_MIDDLE] = mouseButtons & SDL_BUTTON(SDL_BUTTON_MIDDLE);
}

void InputManager::UpdateKeyboardState()
{
    const Uint8* currentKeyStates = SDL_GetKeyboardState(nullptr);

    // Clear previous frame's pressed and released maps
    m_KeyPressedMap.clear();
    m_KeyReleasedMap.clear();

    // Loop through all possible scancodes
    for (int key = 0; key < SDL_NUM_SCANCODES; ++key) {
        bool isKeyDown = currentKeyStates[key];

        if (isKeyDown) {
            if (!m_KeyHeldMap[key]) {
                // Key was just pressed this frame
                m_KeyPressedMap[key] = true;
            }
            // Mark the key as held
            m_KeyHeldMap[key] = true;
        }
        else {
            if (m_KeyHeldMap[key]) {
                // Key was just released this frame
                m_KeyReleasedMap[key] = true;
            }
            // Mark the key as not held
            m_KeyHeldMap[key] = false;
        }
    }
}

bool InputManager::IsMouseButtonPressed(int button) const
{
    auto current = m_MouseButtonStateCurrent.find(button);
    auto previous = m_MouseButtonStatePrevious.find(button);

    bool isCurrentDown = current != m_MouseButtonStateCurrent.end() && current->second;
    bool wasPreviousDown = previous != m_MouseButtonStatePrevious.end() && previous->second;

    return isCurrentDown && !wasPreviousDown;
}

bool InputManager::IsMouseButtonHeld(int button) const
{
    auto current = m_MouseButtonStateCurrent.find(button);
    auto previous = m_MouseButtonStatePrevious.find(button);

    bool isCurrentDown = current != m_MouseButtonStateCurrent.end() && current->second;
    bool wasPreviousDown = previous != m_MouseButtonStatePrevious.end() && previous->second;

    return isCurrentDown && wasPreviousDown;
}

bool InputManager::IsMouseButtonReleased(int button) const
{
    auto current = m_MouseButtonStateCurrent.find(button);
    auto previous = m_MouseButtonStatePrevious.find(button);

    bool isCurrentDown = current != m_MouseButtonStateCurrent.end() && current->second;
    bool wasPreviousDown = previous != m_MouseButtonStatePrevious.end() && previous->second;

    return !isCurrentDown && wasPreviousDown;
}

bool InputManager::IsKeyPressed(SDL_Scancode key) const
{
    return m_KeyPressedMap.count(key) > 0 && m_KeyPressedMap.at(key);
}

bool InputManager::IsKeyHeld(SDL_Scancode key) const
{
    return m_KeyHeldMap.count(key) > 0 && m_KeyHeldMap.at(key);
}

bool InputManager::IsKeyReleased(SDL_Scancode key) const
{
    return m_KeyReleasedMap.count(key) > 0 && m_KeyReleasedMap.at(key);
}