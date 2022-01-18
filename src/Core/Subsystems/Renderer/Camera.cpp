#include "Camera.h"

#include "./Renderer.h"
#include "../Input/InputManager.h"

extern Renderer     g_Renderer;
extern InputManager g_InputManager;

void Camera::update(const float dt)
{
    bool isMoving = false;
    isMoving     |= positionMovements(dt);
    isMoving     |= lookAtMovements();

    // Testings
    //position.z = -0.275 + sin(glfwGetTime() / 2.5) * 2.15;
    isMoving = true;
    // End Testings

    if (isMoving || !_init)
    {
        projection    = glm::perspective(glm::radians(FOV), (float)g_Renderer.SCREEN_WIDTH / (float)g_Renderer.SCREEN_HEIGHT, 1.0f / 32.0f, 1024.0f);
        invProjection = glm::inverse(projection);
        view          = glm::lookAt(position, position + front, up);
        _init = true;
    }
}

bool Camera::positionMovements(const float dt)
{
    bool isMoving = false;
    if (g_InputManager.keyIsDown(KEY_W))
    {
        position += front * speed * dt;
        isMoving = true;
    }
    if (g_InputManager.keyIsDown(KEY_S))
    {
        position -= front * speed * dt;
        isMoving = true;
    }
    if (g_InputManager.keyIsDown(KEY_A))
    {
        position -= glm::normalize(glm::cross(front, up)) * speed * dt;
        isMoving = true;
    }
    if (g_InputManager.keyIsDown(KEY_D))
    {
        position += glm::normalize(glm::cross(front, up)) * speed * dt;
        isMoving = true;
    }
    if (g_InputManager.keyIsDown(KEY_LEFT_SHIFT))
    {
        position.y += speed * dt;
        isMoving = true;
    }
    if (g_InputManager.keyIsDown(KEY_LEFT_CONTROL))
    {
        position.y -= speed * dt;
        isMoving = true;
    }
    return isMoving;
}

bool Camera::lookAtMovements()
{
    const glm::vec2 mouseOffset = g_InputManager.mouseOffset;
    if (mouseOffset.x == 0 && mouseOffset.y == 0 && _init)
    {
        return false;
    }
    
    const float sensitivity = 0.5f;
    yaw  += g_InputManager.mouseOffset.x * sensitivity;
    pitch = glm::clamp(pitch - g_InputManager.mouseOffset.y * sensitivity, -89.99f, 89.99f);

    const glm::vec3 dir =
    {
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))
    };
    front = glm::normalize(dir);

    g_InputManager.resetMouseMovements();
    return true;
}
