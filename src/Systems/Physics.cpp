#include "Physics.h"

#include "../Components/Gravity.h"
#include "../Components/RigidBody.h"
#include "../Components/Transform.h"
#include "../Core/Subsystems/ECS/ECSManager.h"

extern ECSManager ECS_MANAGER;

void Physics::Update(float dt)
{
    for (const auto& entity : _entities)
    {
        auto& rigidBody = ECS_MANAGER.GetComponent<RigidBody>(entity);
        auto& transform = ECS_MANAGER.GetComponent<Transform>(entity);
        const auto& gravity = ECS_MANAGER.GetComponent<Gravity>(entity);

        transform.position += rigidBody.velocity * dt;
        rigidBody.velocity += gravity.force * dt;
    }
}
