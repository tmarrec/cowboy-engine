#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

struct Camera
{
    float FOV               = 70.0f;
    float yaw               = 180.0f;
    float pitch             = 0.0f;
    float speed             = 5.0f;
    glm::vec3 front         = {0.0f, 0.0f, -1.0f};
    glm::vec3 up            = {0.0f, 1.0f, 0.0f};
    glm::mat4 projection    = glm::mat4(1.0);
    glm::mat4 invProjection = glm::mat4(1.0);
    glm::mat4 view          = glm::mat4(1.0);
};
