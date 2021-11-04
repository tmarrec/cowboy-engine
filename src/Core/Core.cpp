#include "Core.h"

#include "../Components/Transform.h"

#include "../Systems/Input.h"

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
    const auto physicsSystem = g_ECSManager.registerSystem<Input>();
    const Signature signature = [&](){
        Signature s;
        s.set(g_ECSManager.getComponentType<Transform>());
        return s;
    }();
    g_ECSManager.setSystemSignature<Input>(signature);

    std::vector<Entity> entities(64);

    std::default_random_engine generator;
    std::uniform_real_distribution<float> randPosition(-100.0f, 100.0f);

    for (auto& entity : entities)
    {
        entity = g_ECSManager.createEntity();

        g_ECSManager.addComponent(
            entity,
            Transform
            {
                .position = glm::vec3(10, 10, 10),
                .rotation = glm::vec3(0, 0, 0),
                .scale = glm::vec3(1.0f, 1.0f, 1.0f)
            });
    }

    float dt = 0.0f;

    while (!g_WindowManager.windowShouldClose())
    {
        const auto startTime = std::chrono::high_resolution_clock::now();

        g_WindowManager.pollEvents();

        g_RendererManager.drawFrame();

        physicsSystem->Update(dt);

        const auto stopTime = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<float, std::chrono::seconds::period>(stopTime - startTime).count();
        //INFO("FPS: " << 1.0/dt);
    }

    g_RendererManager.waitIdle();

    return 0;  
}

void Core::RegisterAllComponents() const
{
    g_ECSManager.registerComponent<Transform>();
}

