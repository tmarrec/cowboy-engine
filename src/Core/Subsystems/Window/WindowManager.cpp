#include "WindowManager.h"
#include <GLFW/glfw3.h>

// Init GLFW
WindowManager::WindowManager()
{
    if (glfwInit() != GLFW_TRUE)
    {
        ERROR_EXIT("Failed to initialize GLFW.");
    }

    glfwSetErrorCallback(&WindowManager::glfwError);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    windowInit();
}

// Destroy the window and terminate GLFW instance
void glfwDeleter::operator()(GLFWwindow* window)
{
    glfwSetErrorCallback(nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

// Window closed event
bool WindowManager::windowShouldClose() const
{
	return glfwWindowShouldClose(_glfwWindow.get());
}

// Poll window events
void WindowManager::pollEvents()
{
	glfwPollEvents();
}

// Returns the Vulkan instance extensions required by GLFW
std::pair<const char**, std::uint32_t> WindowManager::windowGetRequiredInstanceExtensions()
{
	std::uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    if (!glfwExtensions)
    {
        ERROR_EXIT("Failed to get required Vulkan instance extensions.");
    }
	return std::pair<const char**, std::uint32_t>(glfwExtensions, glfwExtensionCount);
}

// Create a Vulkan surface for the window
void WindowManager::windowCreateSurface(VkInstance instance, VkSurfaceKHR* surface)
{
	if (glfwCreateWindowSurface(instance, _glfwWindow.get(), nullptr, surface) != VK_SUCCESS)
	{
        ERROR_EXIT("Failed to create GLFW window surface.");
	}
}

// Create the window
void WindowManager::windowInit()
{
	_glfwWindow.reset(glfwCreateWindow(800, 800, "vulkan-testings", nullptr, nullptr));
}

// GLFW error callback
void WindowManager::glfwError(int error, const char* description)
{
    WARNING(description);
}

void WindowManager::windowGetFramebufferSize(std::uint32_t& width, std::uint32_t& height)
{
    int intWidth, intHeight;
    glfwGetFramebufferSize(_glfwWindow.get(), &intWidth, &intHeight);
    width = static_cast<std::uint32_t>(intWidth);
    height = static_cast<std::uint32_t>(intHeight);
}

