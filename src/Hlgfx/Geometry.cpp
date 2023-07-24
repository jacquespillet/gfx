#include "Include/Geometry.h"

namespace hlgfx
{

indexedGeometryBuffers GetTriangleGeometry()
{
    gfx::context *Context = gfx::context::Get();
    indexedGeometryBuffers Result = {};

    vertex Vertices[] =
    {
        {v4f(-1.0f, -1.0f, 0.0f, 0.0f), v4f(0.0f, 0.0f, 1.0f, 0.0f)},
        {v4f(-1.0f,  1.0f, 0.0f, 0.0f), v4f(0.0f, 0.0f, 1.0f, 1.0f)},
        {v4f( 1.0f,  1.0f, 0.0f, 1.0f), v4f(0.0f, 0.0f, 1.0f, 1.0f)},
        {v4f( 1.0f, -1.0f, 0.0f, 1.0f), v4f(0.0f, 0.0f, 1.0f, 0.0f)},
    };
    
    gfx::vertexStreamData VertexStream1 = {};
    VertexStream1
        .SetSize(sizeof(Vertices))
        .SetStride(sizeof(vertex))
        .SetData(&Vertices)
        .SetStreamIndex(0)
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0})
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 1});
    gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
    VertexBufferCreateInfo.Init().AddVertexStream(VertexStream1);
    Result.VertexBuffer = Context->CreateVertexBuffer(VertexBufferCreateInfo);


    uint32_t Indices[] = 
    {
        0, 2, 1,
        0, 3, 2
    };
    Result.IndexBuffer = Context->CreateBuffer(sizeof(Indices), gfx::bufferUsage::IndexBuffer, gfx::memoryUsage::GpuOnly);
    Context->CopyDataToBuffer(Result.IndexBuffer, &Indices, sizeof(Indices), 0);
    
    Result.Count = 6;
    Result.Start=0;
    return Result;
}    
}