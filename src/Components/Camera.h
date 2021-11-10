#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

struct Camera
{
    float FOV;
    float yaw;
    float pitch;
    float speed;
    glm::vec3 front = {0.0f, 0.0f, 1.0f};
    glm::vec3 up = {0.0f, 1.0f, 0.0f};
    glm::mat4 proj = glm::zero<glm::mat4>();
};
