#pragma once

#include "./../../utils.h"

#include <GLFW/glfw3.h>
#include <map>
#include <iostream>
#include <glm/glm.hpp>

enum InputKey
{
    UNDEFINED,
    KEY_W,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_LEFT_SHIFT,
    KEY_LEFT_CONTROL,
};

class InputManager
{
 public:
    static bool keyIsDown(const InputKey key);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mobs);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void updateMouseMovements();
    static glm::vec2 mouseOffset;
    
 private:
    static std::map<InputKey, bool> _keysStatus;
    static bool _focused;
    static glm::vec2 _lastMousePos;
};
