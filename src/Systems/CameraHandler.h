#pragma once

#include "../Core/Subsystems/ECS/System.h"

#include "../Components/Transform.h"
#include "../Components/Camera.h"


class CameraHandler : public System
{
 public:
    void Update(const float dt);
    const Camera& camera() const;
    const Transform& transform() const;

 private:
    bool positionMovements(Transform& transform, const Camera& camera, const float dt);
    bool lookAtMovements(Camera& camera);
    bool _init = false;
};
