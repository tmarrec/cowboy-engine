#pragma once

#include "../Core/Subsystems/ECS/System.h"

class Input : public System
{
 public:
    void Update(float dt);
};
