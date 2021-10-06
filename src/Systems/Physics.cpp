#include "Physics.h"

#include "../Components/Gravity.h"
#include "../Components/RigidBody.h"
#include "../Components/Transform.h"
#include "../Core/Coordinator.h"

extern Coordinator COORDINATOR;

void Physics::Update(float dt)
{
    for (const auto& entity : _entities)
    {
        auto& rigidBody = COORDINATOR.GetComponent<RigidBody>(entity);
        auto& transform = COORDINATOR.GetComponent<Transform>(entity);
        const auto& gravity = COORDINATOR.GetComponent<Gravity>(entity);

        transform.position += rigidBody.velocity * dt;
        rigidBody.velocity += gravity.force * dt;
    }
}
