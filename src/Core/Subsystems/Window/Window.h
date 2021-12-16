#pragma once

#include "../../utils.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <memory>

struct glfwDeleter
{
    void operator()(GLFWwindow* window);
};

class Window
{
 public:
    Window();
    bool windowShouldClose() const;
    void pollEvents();
    void swapBuffers();
    void windowGetFramebufferSize(uint32_t& width, uint32_t& height);

 private:
    void windowInit();
    static void glfwError(int error, const char* description);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    std::unique_ptr<GLFWwindow, glfwDeleter> _glfwWindow = nullptr;
};
