#pragma once
#include "Object3D.h"
#include "Geometry.h"
#include "Gfx/Api.h"

namespace hlgfx
{
struct material;
struct mesh : public object3D
{
    std::shared_ptr<material> Material;

    mesh();

    void SetVertexBuffer(gfx::vertexBufferHandle VertexBufferHandle);

    virtual void OnRender() override;

    //Api data
    indexedGeometryBuffers GeometryBuffers;
};
}