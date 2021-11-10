#include "Window.h"
#include <GLFW/glfw3.h>

#include "../Renderer/Renderer.h"
#include "../Input/InputManager.h"

extern Renderer g_Renderer;
extern InputManager g_InputManager;

// Init GLFW
Window::Window()
{
    if (glfwInit() != GLFW_TRUE)
    {
        ERROR_EXIT("Failed to initialize GLFW.");
    }

    glfwSetErrorCallback(&Window::glfwError);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    windowInit();

    glfwSetKeyCallback(_glfwWindow.get(), g_InputManager.keyCallback);
    glfwSetCursorPosCallback(_glfwWindow.get(), g_InputManager.cursorPositionCallback);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(_glfwWindow.get(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }
}

// Destroy the window and terminate GLFW instance
void glfwDeleter::operator()(GLFWwindow* window)
{
    glfwSetErrorCallback(nullptr);
	glfwDestroyWindow(window);
	glfwTerminate();
}

// Window closed event
bool Window::windowShouldClose() const
{
	return glfwWindowShouldClose(_glfwWindow.get());
}

// Poll window events
void Window::pollEvents()
{
	glfwPollEvents();
}

// Returns the Vulkan instance extensions required by GLFW
std::pair<const char**, uint32_t> Window::windowGetRequiredInstanceExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    if (!glfwExtensions)
    {
        ERROR_EXIT("Failed to get required Vulkan instance extensions.");
    }
	return std::pair<const char**, uint32_t>(glfwExtensions, glfwExtensionCount);
}

// Create a Vulkan surface for the window
void Window::windowCreateSurface(VkInstance instance, VkSurfaceKHR* surface)
{
	if (glfwCreateWindowSurface(instance, _glfwWindow.get(), nullptr, surface) != VK_SUCCESS)
	{
        ERROR_EXIT("Failed to create GLFW window surface.");
	}
}

// Create the window
void Window::windowInit()
{
	_glfwWindow.reset(glfwCreateWindow(800, 800, "vulkan-testings", nullptr, nullptr));
}

// GLFW error callback
void Window::glfwError(int error, const char* description)
{
    WARNING(description);
}

void Window::windowGetFramebufferSize(uint32_t& width, uint32_t& height)
{
    int intWidth, intHeight;
    glfwGetFramebufferSize(_glfwWindow.get(), &intWidth, &intHeight);
    width = static_cast<uint32_t>(intWidth);
    height = static_cast<uint32_t>(intHeight);
}

void Window::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
    {
        g_Renderer.updateGraphicsPipeline();
    }
}
