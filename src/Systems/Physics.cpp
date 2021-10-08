#include "Physics.h"

#include "../Components/Gravity.h"
#include "../Components/RigidBody.h"
#include "../Components/Transform.h"
#include "../Core/Subsystems/ECS/ECSManager.h"

extern ECSManager g_ECSManager;

void Physics::Update(float dt)
{
    for (const auto& entity : _entities)
    {
        auto& rigidBody = g_ECSManager.getComponent<RigidBody>(entity);
        auto& transform = g_ECSManager.getComponent<Transform>(entity);
        const auto& gravity = g_ECSManager.getComponent<Gravity>(entity);

        transform.position += rigidBody.velocity * dt;
        rigidBody.velocity += gravity.force * dt;
    }
}
