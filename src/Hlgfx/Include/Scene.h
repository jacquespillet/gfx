#pragma once
#include "Object3D.h"
#include "Camera.h"
#include "Gfx/Include/Uniform.h"

namespace hlgfx
{

struct scene : public object3D
{
    scene();
    virtual void OnRender(std::shared_ptr<camera> Camera);
    
    
};

}