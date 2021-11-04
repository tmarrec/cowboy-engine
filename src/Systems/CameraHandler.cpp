#include "CameraHandler.h"

#include "../Core/Subsystems/ECS/ECSManager.h"
#include "../Core/Subsystems/Renderer/RendererManager.h"
#include "../Core/Subsystems/Input/InputManager.h"

extern ECSManager g_ECSManager;
extern RendererManager g_RendererManager;
extern InputManager g_InputManager;

void CameraHandler::Update(const float dt)
{
    for (const auto& entity : _entities)
    {
        auto& transform = g_ECSManager.getComponent<Transform>(entity);
        auto& camera = g_ECSManager.getComponent<Camera>(entity);

        positionMovements(transform, camera, dt);
        lookAtMovements(camera);

        g_RendererManager.setCameraParameters(transform.position, camera.FOV, camera.front, camera.up);
    }
}

void CameraHandler::positionMovements(Transform& transform, const Camera& camera, const float dt)
{
    if (g_InputManager.keyIsDown(KEY_W))
    {
        transform.position += camera.front * camera.speed * dt;
    }
    if (g_InputManager.keyIsDown(KEY_S))
    {
        transform.position -= camera.front * camera.speed * dt;
    }
    if (g_InputManager.keyIsDown(KEY_A))
    {
        transform.position -= glm::normalize(glm::cross(camera.front, camera.up)) * camera.speed * dt;
    }
    if (g_InputManager.keyIsDown(KEY_D))
    {
        transform.position += glm::normalize(glm::cross(camera.front, camera.up)) * camera.speed * dt;
    }
    if (g_InputManager.keyIsDown(KEY_LEFT_SHIFT))
    {
        transform.position.y += camera.speed * dt;
    }
    if (g_InputManager.keyIsDown(KEY_LEFT_CONTROL))
    {
        transform.position.y -= camera.speed * dt;
    }
}

void CameraHandler::lookAtMovements(Camera& camera)
{
    const float sensitivity = 1.0f;
    camera.yaw += g_InputManager.mouseOffset.x * sensitivity;
    camera.pitch = glm::clamp(camera.pitch - g_InputManager.mouseOffset.y * sensitivity, -89.9f, 89.9f);

    glm::vec3 dir;
    dir.x = cos(glm::radians(camera.yaw))*cos(glm::radians(camera.pitch));
    dir.y = sin(glm::radians(camera.pitch));
    dir.z = sin(glm::radians(camera.yaw))*cos(glm::radians(camera.pitch));
    dir = glm::normalize(dir);
    camera.front = dir;

    g_InputManager.updateMouseMovements();
}
