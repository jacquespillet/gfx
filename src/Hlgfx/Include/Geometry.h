#pragma once
#include "Gfx/Include/Types.h"
#include "Gfx/Include/Context.h"
#include "Gfx/Include/Buffer.h"

#include "Types.h"

namespace hlgfx
{

struct vertex
{
    v4f PositionUvX;
    v4f NormalUvY; 
};

struct indexedGeometryBuffers
{
    indexedGeometryBuffers();
    gfx::vertexBufferHandle VertexBuffer = gfx::InvalidHandle;
    gfx::bufferHandle IndexBuffer = gfx::InvalidHandle;
    u32 Start = 0;
    u32 Count = 0;

    std::vector<vertex> VertexData;
    std::vector<u32> IndexData;

    std::string Name;
    std::string UUID;

    void BuildBuffers();
};


std::shared_ptr<indexedGeometryBuffers>  GetTriangleGeometry();

std::shared_ptr<indexedGeometryBuffers>  CreateGeometryFromBuffers(void *VertexBufferData, sz VertexBufferSize, void *IndexBufferData, sz IndexBufferSize);

}