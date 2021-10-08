#include "Core.h"

#include "../Components/Gravity.h"
#include "../Components/RigidBody.h"
#include "../Components/Transform.h"

#include "../Systems/Physics.h"

#include "Subsystems/ECS/ECSManager.h"
#include "Subsystems/Window/WindowManager.h"

#include <random>

// Entity Component System Manager
ECSManager ECS_MANAGER;
// Window Manager
WindowManager WINDOW_MANAGER;

int Core::Run()
{
    RegisterAllComponents();

    auto physicsSystem = ECS_MANAGER.RegisterSystem<Physics>();
    Signature signature;
    signature.set(ECS_MANAGER.GetComponentType<Gravity>());
    signature.set(ECS_MANAGER.GetComponentType<RigidBody>());
    signature.set(ECS_MANAGER.GetComponentType<Transform>());
    ECS_MANAGER.SetSystemSignature<Physics>(signature);

    std::vector<Entity> entities(64);

    std::default_random_engine generator;
    std::uniform_real_distribution<float> randPosition(-100.0f, 100.0f);
    std::uniform_real_distribution<float> randRotation(0.0f, 3.0f);
	std::uniform_real_distribution<float> randGravity(-10.0f, -1.0f);

    for (auto& entity : entities)
    {
        entity = ECS_MANAGER.CreateEntity();

        ECS_MANAGER.AddComponent(
            entity,
            Gravity{glm::vec3(0.0f, randGravity(generator), 0.0f)});

        ECS_MANAGER.AddComponent(
            entity,
            RigidBody{
                .velocity = glm::vec3(0.0f),
                .acceleration = glm::vec3(0.0f)
            });

        ECS_MANAGER.AddComponent(
            entity,
            Transform{
                .position = glm::vec3(randPosition(generator)),
                .rotation = glm::vec3(randRotation(generator)),
                .scale = glm::vec3(1.0f, 1.0f, 1.0f)
            });
    }

    float dt = 0.0f;

    while (!WINDOW_MANAGER.WindowShouldClose())
    {
        auto startTime = std::chrono::high_resolution_clock::now();

        WINDOW_MANAGER.PollEvents();

        physicsSystem->Update(dt);

        auto stopTime = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<float, std::chrono::seconds::period>(stopTime - startTime).count();
    }

    return 0;  
}

void Core::RegisterAllComponents() const
{
    ECS_MANAGER.RegisterComponent<Gravity>();
    ECS_MANAGER.RegisterComponent<RigidBody>();
    ECS_MANAGER.RegisterComponent<Transform>();
}

