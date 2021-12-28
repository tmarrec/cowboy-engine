#include "Window.h"

#include "../Renderer/Renderer.h"
#include "../Input/InputManager.h"


extern Renderer     g_Renderer;
extern InputManager g_InputManager;

// Init GLFW
Window::Window()
{
    if (glfwInit() != GLFW_TRUE)
    {
        ERROR_EXIT("Failed to initialize GLFW.");
    }

    glfwSetErrorCallback(&Window::glfwError);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    windowInit();

    glfwSwapInterval(0);

    glfwSetKeyCallback(_glfwWindow.get(), g_InputManager.keyCallback);
    glfwSetCursorPosCallback(_glfwWindow.get(), g_InputManager.cursorPositionCallback);
    if (glfwRawMouseMotionSupported())
    {
        glfwSetInputMode(_glfwWindow.get(), GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0)
    {
        ERROR_EXIT("Failed to initialize OpenGL context");
    }

    glViewport(0, 0, 800, 800);

    OK("OpenGL");
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

// SwapBuffers
void Window::swapBuffers()
{
    glfwSwapBuffers(_glfwWindow.get());
}

// Create the window
void Window::windowInit()
{
	_glfwWindow.reset(glfwCreateWindow(800, 800, "OpenGL Testings", nullptr, nullptr));
    glfwMakeContextCurrent(_glfwWindow.get());
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
        //g_Renderer.updateGraphicsPipeline();
    }
}
