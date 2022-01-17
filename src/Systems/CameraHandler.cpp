#include "CameraHandler.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "../Core/Subsystems/ECS/ECSManager.h"
#include "../Core/Subsystems/Renderer/Renderer.h"
#include "../Core/Subsystems/Input/InputManager.h"

extern ECSManager g_ECSManager;
extern Renderer g_Renderer;
extern InputManager g_InputManager;

void CameraHandler::Update(const float dt)
{
    for (const auto& entity : _entities)
    {
        auto& transform = g_ECSManager.getComponent<Transform>(entity);
        auto& camera = g_ECSManager.getComponent<Camera>(entity);

        positionMovements(transform, camera, dt);
        lookAtMovements(camera);

        transform.position.z = -0.275 + sin(glfwGetTime()/2.5)*2.15;

        camera.projection = glm::perspective(glm::radians(camera.FOV), (float)g_Renderer.SCREEN_WIDTH / (float)g_Renderer.SCREEN_HEIGHT, 1.0f/32.0f, 1024.0f);
        camera.invProjection = glm::inverse(camera.projection);
        camera.view = glm::lookAt(transform.position, transform.position + camera.front, camera.up);
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
    const float sensitivity = 0.5f;
    camera.yaw += g_InputManager.mouseOffset.x * sensitivity;
    camera.pitch = glm::clamp(camera.pitch - g_InputManager.mouseOffset.y * sensitivity, -89.99f, 89.99f);

    const glm::vec3 dir =
    {
        cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch)),
        sin(glm::radians(camera.pitch)),
        sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch))
    };
    camera.front = glm::normalize(dir);

    g_InputManager.updateMouseMovements();
}

const Camera& CameraHandler::camera() const
{
    for (const auto& entity : _entities)
    {
        auto& camera = g_ECSManager.getComponent<Camera>(entity);
        return camera;
    }
}

const Transform& CameraHandler::transform() const
{
    for (const auto& entity : _entities)
    {
        auto& transform = g_ECSManager.getComponent<Transform>(entity);
        return transform;
    }
}