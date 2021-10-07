#include "Window.h"
#include <GLFW/glfw3.h>

// Init GLFW
Window::Window()
{
    if (glfwInit() != GLFW_TRUE)
    {
        ERROR("Failed to initialize GLFW.");
    }

    glfwSetErrorCallback(&Window::GlfwError);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    WindowInit();
}

// Window closed event
bool Window::WindowShouldClose() const
{
	return glfwWindowShouldClose(_glfwWindow.get());
}

// Poll window events
void Window::PollEvents()
{
	glfwPollEvents();
}

// Returns the Vulkan instance extensions required by GLFW
std::pair<const char**, std::uint32_t> Window::WindowGetRequiredInstanceExtensions()
{
	std::uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	return std::pair<const char**, std::uint32_t>(glfwExtensions, glfwExtensionCount);
}

// Create a Vulkan surface for the window
void Window::WindowCreateSurface(VkInstance instance, VkSurfaceKHR* surface)
{
	if (glfwCreateWindowSurface(instance, _glfwWindow.get(), nullptr, surface) != VK_SUCCESS)
	{
        ERROR("Failed to create GLFW window surface.");
	}
}

// Create the window
void Window::WindowInit()
{
	_glfwWindow.reset(glfwCreateWindow(800, 800, "vulkan-testings", nullptr, nullptr));
}

// GLFW error callback
void Window::GlfwError(int error, const char* description)
{
    WARNING(description);
}

// Destroy the window and terminate GLFW instance
void GlfwDeleter::operator()(GLFWwindow* window)
{
    glfwSetErrorCallback(nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}
