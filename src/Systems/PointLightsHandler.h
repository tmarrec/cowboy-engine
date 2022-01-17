#pragma once

#include "../Core/Subsystems/ECS/System.h"

#include "../Components/Transform.h"
#include "../Components/PointLight.h"


class PointLightsHandler : public System
{
 public:
    void Update(const float dt);
    std::set<Entity>& pointLights();
};
