#pragma once

#include "utils.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

struct GlfwDeleter
{
    void operator()(GLFWwindow* window);
};

class Window
{
 public:
    Window();
    bool WindowShouldClose() const;
    void PollEvents();
    std::pair<const char**, std::uint32_t> WindowGetRequiredInstanceExtensions();
    void WindowCreateSurface(VkInstance instance, VkSurfaceKHR* surface);

 private:
    void WindowInit();
    static void GlfwError(int error, const char* description);

    std::unique_ptr<GLFWwindow, GlfwDeleter> _glfwWindow = nullptr;
};
