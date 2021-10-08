#pragma once

#include "../Core/Subsystems/ECS/System.h"

class Physics : public System
{
 public:
    void Update(float dt);
};
