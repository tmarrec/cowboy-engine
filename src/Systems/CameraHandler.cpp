#include "CameraHandler.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

#include "../Core/Subsystems/ECS/ECSManager.h"
#include "../Core/Subsystems/Renderer/Renderer.h"
#include "../Core/Subsystems/Input/InputManager.h"

extern ECSManager   g_ECSManager;
extern Renderer     g_Renderer;
extern InputManager g_InputManager;

void CameraHandler::Update(const float dt)
{
    // Only using the first camera
    auto& transform = g_ECSManager.getComponent<Transform>(*_entities.begin());
    auto& camera    = g_ECSManager.getComponent<Camera>(*_entities.begin());

    bool isMoving = false;
    isMoving     |= positionMovements(transform, camera, dt);
    isMoving     |= lookAtMovements(camera);

    // Testings
    transform.position.z = -0.275 + sin(glfwGetTime() / 2.5) * 2.15;
    isMoving = true;
    // End Testings

    if (isMoving || !_init)
    {
        camera.projection    = glm::perspective(glm::radians(camera.FOV), (float)g_Renderer.SCREEN_WIDTH / (float)g_Renderer.SCREEN_HEIGHT, 1.0f / 32.0f, 1024.0f);
        camera.invProjection = glm::inverse(camera.projection);
        camera.view          = glm::lookAt(transform.position, transform.position + camera.front, camera.up);
        _init = true;
    }
}

bool CameraHandler::positionMovements(Transform& transform, const Camera& camera, const float dt)
{
    bool isMoving = false;
    if (g_InputManager.keyIsDown(KEY_W))
    {
        transform.position += camera.front * camera.speed * dt;
        isMoving = true;
    }
    if (g_InputManager.keyIsDown(KEY_S))
    {
        transform.position -= camera.front * camera.speed * dt;
        isMoving = true;
    }
    if (g_InputManager.keyIsDown(KEY_A))
    {
        transform.position -= glm::normalize(glm::cross(camera.front, camera.up)) * camera.speed * dt;
        isMoving = true;
    }
    if (g_InputManager.keyIsDown(KEY_D))
    {
        transform.position += glm::normalize(glm::cross(camera.front, camera.up)) * camera.speed * dt;
        isMoving = true;
    }
    if (g_InputManager.keyIsDown(KEY_LEFT_SHIFT))
    {
        transform.position.y += camera.speed * dt;
        isMoving = true;
    }
    if (g_InputManager.keyIsDown(KEY_LEFT_CONTROL))
    {
        transform.position.y -= camera.speed * dt;
        isMoving = true;
    }
    return isMoving;
}

bool CameraHandler::lookAtMovements(Camera& camera)
{
    const glm::vec2 mouseOffset = g_InputManager.mouseOffset;
    if (mouseOffset.x == 0 && mouseOffset.y == 0 && _init)
    {
        return false;
    }
    
    const float sensitivity = 0.5f;
    camera.yaw  += g_InputManager.mouseOffset.x * sensitivity;
    camera.pitch = glm::clamp(camera.pitch - g_InputManager.mouseOffset.y * sensitivity, -89.99f, 89.99f);

    const glm::vec3 dir =
    {
        cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch)),
        sin(glm::radians(camera.pitch)),
        sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch))
    };
    camera.front = glm::normalize(dir);

    g_InputManager.resetMouseMovements();
    return true;
}

const Camera& CameraHandler::camera() const
{
    return g_ECSManager.getComponent<Camera>(*_entities.begin());
}

const Transform& CameraHandler::transform() const
{
    return g_ECSManager.getComponent<Transform>(*_entities.begin());
}