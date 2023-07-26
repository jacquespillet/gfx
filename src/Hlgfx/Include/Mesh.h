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

    virtual void OnRender(std::shared_ptr<camera> Camera) override;
    virtual std::vector<u8> Serialize() override;

    //Api data
    indexedGeometryBuffers GeometryBuffers;

    struct uniformData
    {
        m4x4 ModelMatrix;
    } UniformData;

    gfx::bufferHandle UniformBuffer;
    std::shared_ptr<gfx::uniformGroup> Uniforms;
};
}