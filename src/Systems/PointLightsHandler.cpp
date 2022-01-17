#include "PointLightsHandler.h"

#include "../Core/Subsystems/ECS/ECSManager.h"
#include "../Core/Subsystems/Renderer/Renderer.h"

extern ECSManager   g_ECSManager;
extern Renderer     g_Renderer;

void PointLightsHandler::Update(const float dt)
{
    for (const auto& entity : _entities)
    {
        auto& transform = g_ECSManager.getComponent<Transform>(entity);
        auto& light     = g_ECSManager.getComponent<PointLight>(entity);

        transform.position.y += (light.color.r / 3.0) * dt;
        if (transform.position.y > 25)
        {
            transform.position.y = -5;
        }
    }
    
}

std::set<Entity>& PointLightsHandler::pointLights()
{
    return _entities;
}