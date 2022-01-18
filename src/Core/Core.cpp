#include "Core.h"

#include "../Components/Transform.h"
#include "../Components/PointLight.h"

#include "../Systems/PointLightsHandler.h"

#include "Subsystems/ECS/ECSManager.h"
#include "Subsystems/Window/Window.h"
#include "Subsystems/Renderer/Renderer.h"
#include "Subsystems/Renderer/Camera.h"

#include <random>

ECSManager          g_ECSManager;
Window              g_Window;
Renderer            g_Renderer;
Camera              g_Camera;
auto                g_PointLights = g_ECSManager.registerSystem<PointLightsHandler>();

int Core::Run()
{
    RegisterAllComponents();

    // Light system initialization
    const Signature pointLightsSignature = [&]() {
        Signature s;
        s.set(g_ECSManager.getComponentType<Transform>());
        s.set(g_ECSManager.getComponentType<PointLight>());
        return s;
    }();
    g_ECSManager.setSystemSignature<PointLightsHandler>(pointLightsSignature);

    std::vector<Entity> lightEntities(50000);

    float lightIntensity = 1.0f;
    std::default_random_engine genR;
    std::uniform_real_distribution<float> randX(-16.0f, 15.0f);
    std::uniform_real_distribution<float> randZ(-10.0f, 9.5f);
    std::uniform_real_distribution<float> randColor(0.0f, lightIntensity);
    std::uniform_real_distribution<float> randRange(0.2f, 0.4f);
    
    for (auto& entity : lightEntities)
    {
        entity = g_ECSManager.createEntity();
        
        g_ECSManager.addComponent
        (
            entity,
            Transform
            {
                .position = {randX(genR), -5, randZ(genR)},
                .rotation = glm::vec3(0, 0, 0),
                .scale = glm::vec3(1.0f, 1.0f, 1.0f)
            }
        );
        g_ECSManager.addComponent
        (
            entity,
            PointLight
            {
                .color = {randColor(genR), randColor(genR), randColor(genR)},
                .range = randRange(genR),
                .position = glm::vec4(1),   // ignored
                .positionVS = glm::vec4(1)  // ignored
            }
        );
    }

    // Important //
    g_Camera.update(0);
    g_Renderer.init();

    float dt = 0.0f;

    while (!g_Window.windowShouldClose())
    {
        const auto startTime = std::chrono::high_resolution_clock::now();

        g_Camera.update(dt);
        g_PointLights->update(dt);

        g_Renderer.drawFrame();

        g_Window.pollEvents();
        g_Window.swapBuffers();

        const auto stopTime = std::chrono::high_resolution_clock::now();
		dt = std::chrono::duration<float, std::chrono::seconds::period>(stopTime - startTime).count();
        INFO("FPS: " << 1.0/dt);
    }

    return EXIT_SUCCESS;  
}

void Core::RegisterAllComponents() const
{
    g_ECSManager.registerComponent<Transform>();
    g_ECSManager.registerComponent<PointLight>();
}

