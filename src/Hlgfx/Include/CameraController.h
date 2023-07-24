#pragma once

#include <memory>
#include "Camera.h"
#include "Object3D.h"

namespace hlgfx
{

struct orbitCameraController : public object3D
{
    orbitCameraController(std::shared_ptr<camera> Camera);
    std::shared_ptr<camera> Camera;

    virtual void OnUpdate() override;

    f32 Theta = 0.0f;
    f32 Phi = 0.0f;
    f32 Distance = 1.0f;

    v3f Target = v3f(0,0,0);

    f32 MouseSpeedX = 1.0f;
    f32 MouseSpeedY = 1.0f;
    f32 MouseSpeedWheel = 1.0f;
};

}