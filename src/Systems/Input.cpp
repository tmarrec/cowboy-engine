#include "Input.h"

#include "../Core/Subsystems/ECS/ECSManager.h"
#include "../Core/Subsystems/Input/InputManager.h"
#include "../Components/Transform.h"
#include <GLFW/glfw3.h>

extern ECSManager g_ECSManager;
extern InputManager g_InputManager;

void Input::Update(float dt)
{
    for (const auto& entity : _entities)
    {
        auto& transform = g_ECSManager.getComponent<Transform>(entity);

        if (g_InputManager.keyIsDown(KEY_W))
        {
            transform.position.x += 1 * dt;
        }
        if (g_InputManager.keyIsDown(KEY_S))
        {
            transform.position.x -= 1 * dt;
        }
        if (g_InputManager.keyIsDown(KEY_A))
        {
            transform.position.z -= 1 * dt;
        }
        if (g_InputManager.keyIsDown(KEY_D))
        {
            transform.position.z += 1 * dt;
        }
        std::cout << transform.position.x << ' ' << transform.position.z << '\n';
    }
}
