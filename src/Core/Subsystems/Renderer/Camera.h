#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

class Camera
{
 public:
    void update(const float dt);

    float FOV   = 70.0f;
    float yaw   = 180.0f;
    float pitch = 0.0f;
    float speed = 10.0f;

    glm::vec3 position         = { 9.5f, 5.25f, -0.275f };
    glm::vec3 front            = { 0.0f, 0.0f, -1.0f };
    glm::vec3 up               = { 0.0f, 1.0f, 0.0f };
    glm::mat4 projection       = glm::mat4(1.0);
    glm::mat4 invProjection    = glm::mat4(1.0);
    glm::mat4 view             = glm::mat4(1.0);

 private:
    bool positionMovements(const float dt);
    bool lookAtMovements();
    bool _init = false;
};
