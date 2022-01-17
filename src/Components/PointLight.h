#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

struct PointLight
{
    glm::vec3   color;
    float       range;
    glm::vec4   position;
    glm::vec4   positionVS;
};
