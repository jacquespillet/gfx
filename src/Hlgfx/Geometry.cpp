#include "Include/Geometry.h"

namespace hlgfx
{

void indexedGeometryBuffers::BuildBuffers()
{
    gfx::context *Context = gfx::context::Get();
    
    gfx::vertexStreamData VertexStream1 = {};
    VertexStream1
        .SetSize(this->VertexData.size() * sizeof(vertex))
        .SetStride(sizeof(vertex))
        .SetData(this->VertexData.data())
        .SetStreamIndex(0)
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0})
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 1});
    gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
    VertexBufferCreateInfo.Init().AddVertexStream(VertexStream1);
    this->VertexBuffer = Context->CreateVertexBuffer(VertexBufferCreateInfo);

    this->IndexBuffer = Context->CreateBuffer(this->IndexData.size() * sizeof(u32), gfx::bufferUsage::IndexBuffer, gfx::memoryUsage::GpuOnly);
    Context->CopyDataToBuffer(this->IndexBuffer, this->IndexData.data(), this->IndexData.size() * sizeof(u32), 0);
    
    this->Count = this->IndexData.size();
    this->Start=0;  
}

std::shared_ptr<indexedGeometryBuffers> CreateGeometryFromBuffers(void *VertexBufferData, sz VertexBufferSize, void *IndexBufferData, sz IndexBufferSize)
{
    gfx::context *Context = gfx::context::Get();
    std::shared_ptr<indexedGeometryBuffers> Result = std::make_shared<indexedGeometryBuffers>();
    
    gfx::vertexStreamData VertexStream1 = {};
    VertexStream1
        .SetSize(VertexBufferSize)
        .SetStride(sizeof(vertex))
        .SetData(VertexBufferData)
        .SetStreamIndex(0)
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0})
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 1});
    gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
    VertexBufferCreateInfo.Init().AddVertexStream(VertexStream1);
    Result->VertexBuffer = Context->CreateVertexBuffer(VertexBufferCreateInfo);

    Result->IndexBuffer = Context->CreateBuffer(IndexBufferSize, gfx::bufferUsage::IndexBuffer, gfx::memoryUsage::GpuOnly);
    Context->CopyDataToBuffer(Result->IndexBuffer, IndexBufferData, IndexBufferSize, 0);
    
    Result->VertexData.resize(VertexBufferSize / sizeof(vertex));
    Result->IndexData.resize(IndexBufferSize / sizeof(u32));
    memcpy(Result->VertexData.data(), VertexBufferData, VertexBufferSize);
    memcpy(Result->IndexData.data(), IndexBufferData, IndexBufferSize);

    Result->Count = Result->IndexData.size();
    Result->Start=0;
    return Result;   
}


std::shared_ptr<indexedGeometryBuffers>  GetTriangleGeometry()
{
    gfx::context *Context = gfx::context::Get();
    std::shared_ptr<indexedGeometryBuffers> Result = std::make_shared<indexedGeometryBuffers>();

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
    Result->VertexBuffer = Context->CreateVertexBuffer(VertexBufferCreateInfo);


    uint32_t Indices[] = 
    {  
        0, 2, 1,
        0, 3, 2
    };
    Result->IndexBuffer = Context->CreateBuffer(sizeof(Indices), gfx::bufferUsage::IndexBuffer, gfx::memoryUsage::GpuOnly);
    Context->CopyDataToBuffer(Result->IndexBuffer, &Indices, sizeof(Indices), 0);
    
    Result->VertexData.resize(sizeof(Vertices));
    memcpy(Result->VertexData.data(), &Vertices, sizeof(Vertices));
    Result->IndexData.resize(sizeof(Indices));
    memcpy(Result->IndexData.data(), &Indices, sizeof(Indices));

    Result->Count = 6;
    Result->Start=0;
    return Result;
}    
}