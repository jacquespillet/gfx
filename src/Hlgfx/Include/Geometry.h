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
    gfx::vertexBufferHandle VertexBuffer;
    gfx::bufferHandle IndexBuffer;
    u32 Start, Count;
};


indexedGeometryBuffers GetTriangleGeometry();

}