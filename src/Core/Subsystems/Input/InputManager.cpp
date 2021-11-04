#include "InputManager.h"

std::map<InputKey, bool> InputManager::_keysStatus;
bool InputManager::_focused = false;
glm::vec2 InputManager::_lastMousePos;
glm::vec2 InputManager::mouseOffset;

static inline InputKey toGLFWType(const int key)
{
    switch (key)
    {
        case GLFW_KEY_W:
            return KEY_W;
        case GLFW_KEY_A:
            return KEY_A;
        case GLFW_KEY_S:
            return KEY_S;
        case GLFW_KEY_D:
            return KEY_D;
        case GLFW_KEY_LEFT_SHIFT:
            return KEY_LEFT_SHIFT;
        case GLFW_KEY_LEFT_CONTROL:
            return KEY_LEFT_CONTROL;
        default:
            return UNDEFINED;
    }
}

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mobs)
{
    InputKey realKey = toGLFWType(key);
    switch(action)
    {
        case GLFW_PRESS:
            _keysStatus[realKey] = true;
            break;
        case GLFW_RELEASE:
            _keysStatus[realKey] = false;
            break;
    }
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        _focused = false;
    }
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        if (!_focused)
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            _focused = true;
        }
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            _focused = false;
        }
    }
}

bool InputManager::keyIsDown(const InputKey key)
{
    std::map<InputKey, bool>::iterator it = _keysStatus.find(key);
    if (it != _keysStatus.end())
    {
        return _keysStatus[key];
    }
    return false;
}

void InputManager::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (_focused)
    {
        mouseOffset.x = xpos-_lastMousePos.x;
        mouseOffset.y = ypos-_lastMousePos.y;
    }
    _lastMousePos.x = xpos;
    _lastMousePos.y = ypos;
}

void InputManager::updateMouseMovements()
{
    mouseOffset.x = 0.0f;
    mouseOffset.y = 0.0f;
}
