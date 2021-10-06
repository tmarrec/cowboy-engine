#include "Core.h"
#include "EntityManager.h"

#include "Coordinator.h"
#include "../Components/Gravity.h"
#include "../Components/RigidBody.h"
#include "../Components/Transform.h"
#include "../Systems/Physics.h"

#include <random>

Coordinator COORDINATOR;

int Core::Run()
{
    RegisterAllComponents();

    auto physicsSystem = COORDINATOR.RegisterSystem<Physics>();
    Signature signature;
    signature.set(COORDINATOR.GetComponentType<Gravity>());
    signature.set(COORDINATOR.GetComponentType<RigidBody>());
    signature.set(COORDINATOR.GetComponentType<Transform>());
    COORDINATOR.SetSystemSignature<Physics>(signature);

    std::vector<Entity> entities(MAX_ENTITIES);

    std::default_random_engine generator;
    std::uniform_real_distribution<float> randPosition(-100.0f, 100.0f);
    std::uniform_real_distribution<float> randRotation(0.0f, 3.0f);
	std::uniform_real_distribution<float> randGravity(-10.0f, -1.0f);

    for (auto& entity : entities)
    {
        entity = COORDINATOR.CreateEntity();

        COORDINATOR.AddComponent(
            entity,
            Gravity{glm::vec3(0.0f, randGravity(generator), 0.0f)});

        COORDINATOR.AddComponent(
            entity,
            RigidBody{
                .velocity = glm::vec3(0.0f),
                .acceleration = glm::vec3(0.0f)
            });

        COORDINATOR.AddComponent(
            entity,
            Transform{
                .position = glm::vec3(randPosition(generator)),
                .rotation = glm::vec3(randRotation(generator)),
                .scale = glm::vec3(1.0f, 1.0f, 1.0f)
            });
    }

    float dt = 0.0f;
    
    for (std::uint64_t i = 0; i < 2048; ++i)
    {
        auto startTime = std::chrono::high_resolution_clock::now();

        physicsSystem->Update(dt);

        auto stopTime = std::chrono::high_resolution_clock::now();

		dt = std::chrono::duration<float, std::chrono::seconds::period>(stopTime - startTime).count();
    }

    for (auto& entity : entities)
    {
        COORDINATOR.DestroyEntity(entity);
    }

    return 0;  
}

void Core::RegisterAllComponents() const
{
    COORDINATOR.RegisterComponent<Gravity>();
    COORDINATOR.RegisterComponent<RigidBody>();
    COORDINATOR.RegisterComponent<Transform>();
}

