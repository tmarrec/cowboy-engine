#pragma once

#include "../../utils.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <memory>

struct glfwDeleter
{
    void operator()(GLFWwindow* window);
};

class WindowManager
{
 public:
    WindowManager();
    bool windowShouldClose() const;
    void pollEvents();
    std::pair<const char**, std::uint32_t> windowGetRequiredInstanceExtensions();
    void windowCreateSurface(VkInstance instance, VkSurfaceKHR* surface);

 private:
    void windowInit();
    static void glfwError(int error, const char* description);

    std::unique_ptr<GLFWwindow, glfwDeleter> _glfwWindow = nullptr;
};
