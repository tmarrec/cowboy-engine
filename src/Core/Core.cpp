#include "Core.h"

#include "../Components/Transform.h"
#include "../Components/Camera.h"
#include "../Components/PointLight.h"

#include "../Systems/CameraHandler.h"
#include "../Systems/PointLightsHandler.h"

#include "Subsystems/ECS/ECSManager.h"
#include "Subsystems/Window/Window.h"
#include "Subsystems/Renderer/Renderer.h"

#include <random>

ECSManager          g_ECSManager;
Window              g_Window;
Renderer            g_Renderer;
auto                g_Camera      = g_ECSManager.registerSystem<CameraHandler>();
auto                g_PointLights = g_ECSManager.registerSystem<PointLightsHandler>();

int Core::Run()
{
    RegisterAllComponents();

    // Camera system initialization
    const Signature cameraSignature = [&](){
        Signature s;
        s.set(g_ECSManager.getComponentType<Transform>());
        s.set(g_ECSManager.getComponentType<Camera>());
        return s;
    }();
    g_ECSManager.setSystemSignature<CameraHandler>(cameraSignature);

    // Light system initialization
    const Signature pointLightsSignature = [&]() {
        Signature s;
        s.set(g_ECSManager.getComponentType<Transform>());
        s.set(g_ECSManager.getComponentType<PointLight>());
        return s;
    }();
    g_ECSManager.setSystemSignature<PointLightsHandler>(pointLightsSignature);

    Entity mainEntity = g_ECSManager.createEntity();
    g_ECSManager.addComponent
    (
        mainEntity,
        Transform
        {
            .position = {9.5, 5.25, -0.275},
            .rotation = glm::vec3(0, 0, 0),
            .scale = glm::vec3(1.0f, 1.0f, 1.0f)
        }
    );

    g_ECSManager.addComponent
    (
        mainEntity,
        Camera{}
    );

    std::vector<Entity> entities(32768);
    srand(static_cast <unsigned> (time(0)));
    
    for (auto& entity : entities)
    {
        // should use uniform_real_distribution
        float x = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 23.0 - 23.0 / 2.0;
        float y = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 7 + 0.5f;
        float z = (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * 11.0 - 11.0 / 2.0;

        float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
        float k = 10.0f;

        entity = g_ECSManager.createEntity();
        
        g_ECSManager.addComponent
        (
            entity,
            Transform
            {
                .position = {x, -5, z},
                .rotation = glm::vec3(0, 0, 0),
                .scale = glm::vec3(1.0f, 1.0f, 1.0f)
            }
        );
        g_ECSManager.addComponent
        (
            entity,
            PointLight
            {
                .color = {r * k, g * k, b * k},
                .range = 0.4f,
                .position = glm::vec4(1),   // ignored
                .positionVS = glm::vec4(1)  // ignored
            }
        );
    }

    // Important //
    g_Camera->Update(0);
    g_Renderer.init();

    float dt = 0.0f;

    while (!g_Window.windowShouldClose())
    {
        const auto startTime = std::chrono::high_resolution_clock::now();

        g_Camera->Update(dt);
        g_PointLights->Update(dt);

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
    g_ECSManager.registerComponent<Camera>();
    g_ECSManager.registerComponent<PointLight>();
}

