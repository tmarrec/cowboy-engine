#pragma once

#include <GLFW/glfw3.h>
#include <map>
#include <iostream>

#include "../../utils.h"

enum InputKey
{
    UNDEFINED,
    KEY_W,
    KEY_A,
    KEY_S,
    KEY_D,
};

class InputManager
{
 public:
    static bool keyIsDown(const InputKey key);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mobs);
    
 private:
    static std::map<InputKey, bool> _keysStatus;
    static bool _focused;
};
