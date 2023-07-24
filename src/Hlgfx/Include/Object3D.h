#pragma once

#include <memory>
#include "Types.h"
#include "Transform.h"
#include <vector>

namespace hlgfx
{
struct object3D
{
    object3D(object3D *Parent = nullptr);

    object3D *Parent;
    std::vector<std::shared_ptr<object3D>> Children;
    transform Transform;

    u32 RenderOrder=0;
    b8 FrustumCulled=false;
    b8 CastShadow=false;
    b8 ReceiveShadow=false;
    
    void SetRenderOrder(u32 RenderOrder);
    void SetFrustumCulled(b8 FrustumCulled);
    void SetCastShadow(b8 CastShadow);
    void SetReceiveShadow(b8 ReceiveShadow);

    virtual void AddObject(std::shared_ptr<object3D> Object);
    virtual void OnUpdate();
    virtual void OnBeforeRender();
    virtual void OnRender();
    virtual void OnAfterRender();
};
}