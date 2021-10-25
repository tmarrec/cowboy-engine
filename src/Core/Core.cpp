#include "Core.h"

#include "../Components/Gravity.h"
#include "../Components/RigidBody.h"
#include "../Components/Transform.h"

#include "../Systems/Physics.h"

#include "Subsystems/ECS/ECSManager.h"
#include "Subsystems/Window/WindowManager.h"
#include "Subsystems/Renderer/RendererManager.h"

#include <random>

// Entity Component System Manager
ECSManager g_ECSManager;
// Window Manager
WindowManager g_WindowManager;
// Renderer Manager
RendererManager g_RendererManager;

int Core::Run()
{
    RegisterAllComponents();

    // Physics initialization
    const auto physicsSystem = g_ECSManager.registerSystem<Physics>();
    const Signature signature = [&](){
        Signature s;
        s.set(g_ECSManager.getComponentType<Gravity>());
        s.set(g_ECSManager.getComponentType<RigidBody>());
        s.set(g_ECSManager.getComponentType<Transform>());
        return s;
    }();
    g_ECSManager.setSystemSignature<Physics>(signature);

    std::vector<Entity> entities(64);

    std::default_random_engine generator;
    std::uniform_real_distribution<float> randPosition(-100.0f, 100.0f);
    std::uniform_real_distribution<float> randRotation(0.0f, 3.0f);
	std::uniform_real_distribution<float> randGravity(-10.0f, -1.0f);

    for (auto& entity : entities)
    {
        entity = g_ECSManager.createEntity();

        g_ECSManager.addComponent(
            entity,
            Gravity{glm::vec3(0.0f, randGravity(generator), 0.0f)});

        g_ECSManager.addComponent(
            entity,
            RigidBody{
                .velocity = glm::vec3(0.0f),
                .acceleration = glm::vec3(0.0f)
            });

        g_ECSManager.addComponent(
            entity,
            Transform{
                .position = glm::vec3(randPosition(generator)),
                .rotation = glm::vec3(randRotation(generator)),
                .scale = glm::vec3(1.0f, 1.0f, 1.0f)
            });
    }

    float dt = 0.0f;

    while (!g_WindowManager.windowShouldClose())
    {
        const auto startTime = std::chrono::high_resolution_clock::now();

        g_WindowManager.pollEvents();

        physicsSystem->Update(dt);

        const auto stopTime = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<float, std::chrono::seconds::period>(stopTime - startTime).count();
    }

    return 0;  
}

void Core::RegisterAllComponents() const
{
    g_ECSManager.registerComponent<Gravity>();
    g_ECSManager.registerComponent<RigidBody>();
    g_ECSManager.registerComponent<Transform>();
}

