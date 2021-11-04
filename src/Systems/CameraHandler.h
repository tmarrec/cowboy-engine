#pragma once

#include "../Core/Subsystems/ECS/System.h"

#include "../Components/Transform.h"
#include "../Components/Camera.h"


class CameraHandler : public System
{
 public:
    void Update(const float dt);
 private:
    void positionMovements(Transform& transform, const Camera& camera, const float dt);
    void lookAtMovements(Camera& camera);
};
