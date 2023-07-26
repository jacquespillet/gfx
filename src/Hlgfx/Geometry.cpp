#include "Include/Geometry.h"

namespace hlgfx
{

indexedGeometryBuffers CreateGeometryFromBuffers(std::vector<u8> &VertexBuffer, std::vector<u8> &IndexBuffer)
{
    gfx::context *Context = gfx::context::Get();
    indexedGeometryBuffers Result = {};
    
    gfx::vertexStreamData VertexStream1 = {};
    VertexStream1
        .SetSize(VertexBuffer.size())
        .SetStride(sizeof(vertex))
        .SetData(VertexBuffer.data())
        .SetStreamIndex(0)
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0})
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 1});
    gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
    VertexBufferCreateInfo.Init().AddVertexStream(VertexStream1);
    Result.VertexBuffer = Context->CreateVertexBuffer(VertexBufferCreateInfo);

    Result.IndexBuffer = Context->CreateBuffer(IndexBuffer.size(), gfx::bufferUsage::IndexBuffer, gfx::memoryUsage::GpuOnly);
    Context->CopyDataToBuffer(Result.IndexBuffer, IndexBuffer.data(), IndexBuffer.size(), 0);
    
    Result.VertexData = VertexBuffer;
    Result.IndexData = IndexBuffer;

    Result.Count = IndexBuffer.size() / sizeof(u32);
    Result.Start=0;
    return Result;   
}


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
    
    Result.VertexData.resize(sizeof(Vertices));
    memcpy(Result.VertexData.data(), &Vertices, sizeof(Vertices));
    Result.IndexData.resize(sizeof(Indices));
    memcpy(Result.IndexData.data(), &Indices, sizeof(Indices));

    Result.Count = 6;
    Result.Start=0;
    return Result;
}    
}