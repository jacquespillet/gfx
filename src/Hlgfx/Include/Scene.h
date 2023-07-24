#pragma once
#include "Object3D.h"
#include "Camera.h"
#include "Mesh.h"
#include "Gfx/Include/Uniform.h"

namespace hlgfx
{

struct scene : public object3D
{
    scene();
    virtual void OnRender(std::shared_ptr<camera> Camera);
    virtual void AddObject(std::shared_ptr<object3D> Object) override;
    
    std::unordered_map<gfx::pipelineHandle, std::vector<std::shared_ptr<mesh>>> Meshes;
};

}