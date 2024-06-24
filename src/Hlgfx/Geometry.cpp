#include "Include/Geometry.h"
#include "Include/Context.h"
#include <fstream>

#define PI 3.1415926
#define TWO_PI 6.283185

namespace hlgfx
{
indexedGeometryBuffers::indexedGeometryBuffers()
{
    this->ID = context::Get()->Project.Geometries.size();
}

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
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 1})
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 2});
    gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
    VertexBufferCreateInfo.Init().AddVertexStream(VertexStream1);
    this->VertexBuffer = Context->CreateVertexBuffer(VertexBufferCreateInfo);

    gfx::bufferUsage::Bits IndexBufferUsage = gfx::bufferUsage::IndexBuffer;
    if(Context->RTXEnabled) IndexBufferUsage = (gfx::bufferUsage::Bits)((u32)IndexBufferUsage | (u32)gfx::bufferUsage::ShaderDeviceAddress | (u32)gfx::bufferUsage::AccelerationStructureBuildInputReadonly);

    this->IndexBuffer = Context->CreateBuffer(this->IndexData.size() * sizeof(u32), IndexBufferUsage, gfx::memoryUsage::GpuOnly);
    Context->CopyDataToBuffer(this->IndexBuffer, this->IndexData.data(), this->IndexData.size() * sizeof(u32), 0);
    
#if GFX_API == GFX_VK
    if(Context->RTXEnabled)
    {
        gfx::vertexBuffer *VBuffer = Context->GetVertexBuffer(this->VertexBuffer);
		BLAS = Context->CreateBLAccelerationStructure(this->VertexData.size(), sizeof(vertex), gfx::format::R32G32B32_SFLOAT, VBuffer->VertexStreams[0].Buffer,
                                                      gfx::indexType::Uint32, this->IndexData.size() / 3, this->IndexBuffer, 0);
    }

    

#endif

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
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 1})
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 2});
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

void CalculateTangents(std::vector<vertex>& Vertices, std::vector<u32> Indices) {
    std::vector<glm::vec4> Tangent1(Vertices.size(), glm::vec4(0));
    std::vector<glm::vec4> Tangent2(Vertices.size(), glm::vec4(0));
    for(uint64_t i=0; i<Indices.size()-3; i+=3) {
        glm::vec3 v1 = Vertices[Indices[i]].PositionUvX;
        glm::vec3 v2 = Vertices[Indices[i + 1]].PositionUvX;
        glm::vec3 v3 = Vertices[Indices[i + 2]].PositionUvX;

        glm::vec2 w1 = v2f(Vertices[Indices[i]].PositionUvX.w,Vertices[Indices[i]].NormalUvY.w);
        glm::vec2 w2 = v2f(Vertices[Indices[i+1]].PositionUvX.w,Vertices[Indices[i+1]].NormalUvY.w);
        glm::vec2 w3 = v2f(Vertices[Indices[i+2]].PositionUvX.w,Vertices[Indices[i+2]].NormalUvY.w);

        double x1 = v2.x - v1.x;
        double x2 = v3.x - v1.x;
        double y1 = v2.y - v1.y;
        double y2 = v3.y - v1.y;
        double z1 = v2.z - v1.z;
        double z2 = v3.z - v1.z;

        double s1 = w2.x - w1.x;
        double s2 = w3.x - w1.x;
        double t1 = w2.y - w1.y;
        double t2 = w3.y - w1.y;

        double r = 1.0F / (s1 * t2 - s2 * t1);
        glm::vec4 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r, 0);
        glm::vec4 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r, 0);

        Tangent1[Indices[i]] += sdir;
        Tangent1[Indices[i + 1]] += sdir;
        Tangent1[Indices[i + 2]] += sdir;
        
        Tangent2[Indices[i]] += tdir;
        Tangent2[Indices[i + 1]] += tdir;
        Tangent2[Indices[i + 2]] += tdir;
    }
    for(uint64_t i=0; i<Vertices.size(); i++) { 
		glm::vec3 n = Vertices[i].NormalUvY;
		glm::vec3 t = glm::vec3(Tangent1[i]);

		Vertices[i].Tangent = glm::vec4(glm::normalize((t - n * glm::dot(n, t))), 1);
		Vertices[i].Tangent.w = (glm::dot(glm::cross(n, t), glm::vec3(Tangent2[i])) < 0.0F) ? -1.0F : 1.0F;
	}
}

std::shared_ptr<indexedGeometryBuffers>  GetQuadGeometry()
{
    gfx::context *Context = gfx::context::Get();
    std::shared_ptr<indexedGeometryBuffers> Result = std::make_shared<indexedGeometryBuffers>();

    std::vector<vertex> Vertices =
    {
        {v4f(-1.0f, -1.0f, 0.0f, 0.0f), v4f(0.0f, 0.0f, 1.0f, 0.0f)},
        {v4f(-1.0f,  1.0f, 0.0f, 0.0f), v4f(0.0f, 0.0f, 1.0f, 1.0f)},
        {v4f( 1.0f,  1.0f, 0.0f, 1.0f), v4f(0.0f, 0.0f, 1.0f, 1.0f)},
        {v4f( 1.0f, -1.0f, 0.0f, 1.0f), v4f(0.0f, 0.0f, 1.0f, 0.0f)},
    };
    std::vector<uint32_t> Indices = 
    {  
        0, 2, 1,
        0, 3, 2
    };

    CalculateTangents(Vertices, Indices);
    
    gfx::vertexStreamData VertexStream1 = {};
    VertexStream1
        .SetSize(Vertices.size() * sizeof(vertex))
        .SetStride(sizeof(vertex))
        .SetData(Vertices.data())
        .SetStreamIndex(0)
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0})
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 1});
    gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
    VertexBufferCreateInfo.Init().AddVertexStream(VertexStream1);
    Result->VertexBuffer = Context->CreateVertexBuffer(VertexBufferCreateInfo);


    Result->IndexBuffer = Context->CreateBuffer(Indices.size() * sizeof(u32), gfx::bufferUsage::IndexBuffer, gfx::memoryUsage::GpuOnly);
    Context->CopyDataToBuffer(Result->IndexBuffer, Indices.data(), Indices.size() * sizeof(u32), 0);
    
    Result->VertexData = Vertices;
    Result->IndexData = Indices;

    Result->Count = 6;
    Result->Start=0;
    return Result;
}    



std::shared_ptr<indexedGeometryBuffers>  GetCubeGeometry()
{

    gfx::context *Context = gfx::context::Get();
    std::shared_ptr<indexedGeometryBuffers> Result = std::make_shared<indexedGeometryBuffers>();

    std::vector<vertex> Vertices =
    {
        {v4f(-0.5, -0.5, -0.5, 0), v4f(0, 0, -1, 1)},
        {v4f(-0.5, 0.5, -0.5, 0), v4f(0, 0, -1, 0)},
        {v4f(0.5, 0.5, -0.5, 1), v4f(0, 0, -1, 0)},
        {v4f(0.5, -0.5, -0.5, 1), v4f(0, 0, -1, 1)},

        {v4f(0.5, -0.5, 0.5, 0), v4f(0, 0, 1, 1)},
        {v4f(0.5, 0.5, 0.5, 0), v4f(0, 0, 1, 0)},
        {v4f(-0.5, 0.5, 0.5, 1), v4f(0, 0, 1, 0)},
        {v4f(-0.5, -0.5, 0.5, 1), v4f(0, 0, 1, 1)},
        
        {v4f(0.5, -0.5, -0.5, 0), v4f(1, 0, 0, 1)},
        {v4f(0.5, 0.5, -0.5, 0), v4f(1, 0, 0, 0)},
        {v4f(0.5, 0.5, 0.5, 1), v4f(1, 0, 0, 0)},
        {v4f(0.5, -0.5, 0.5, 1), v4f(1, 0, 0, 1)},
        
        {v4f(-0.5, -0.5, 0.5, 0), v4f(-1, 0, 0, 1)},
        {v4f(-0.5, 0.5, 0.5, 0), v4f(-1, 0, 0, 0)},
        {v4f(-0.5, 0.5, -0.5, 1), v4f(-1, 0, 0, 0)},
        {v4f(-0.5, -0.5,-0.5, 1), v4f(-1, 0, 0, 1)},
        
        {v4f(-0.5, 0.5, -0.5, 0), v4f(0, 1, 0, 1)},
        {v4f(-0.5, 0.5, 0.5, 0), v4f(0, 1, 0, 0)},
        {v4f(0.5, 0.5, 0.5, 1), v4f(0, 1, 0, 0)},
        {v4f(0.5, 0.5, -0.5, 1), v4f(0, 1, 0, 1)},
        
        {v4f( -0.5, -0.5,  0.5, 0), v4f(0, -1, 0, 1)},
        {v4f( -0.5, -0.5, -0.5, 0), v4f(0, -1, 0, 0)},
        {v4f(0.5, -0.5, -0.5, 1), v4f(0, -1, 0, 0)},
        {v4f(0.5, -0.5,  0.5, 1), v4f(0, -1, 0, 1)},
    };
    

    std::vector<uint32_t> Indices = 
    {  
        0,  1,  2,  3,  0,  2,
        4,  5,  6,  7,  4,  6,
        8,  9, 10,  11, 8, 10,
        12, 13, 14, 15, 12, 14,
        16, 17, 18, 19, 16, 18,
        20, 21, 22, 23, 20, 22,
    };
    CalculateTangents(Vertices, Indices);

    gfx::vertexStreamData VertexStream1 = {};
    VertexStream1
        .SetSize(Vertices.size() * sizeof(vertex))
        .SetStride(sizeof(vertex))
        .SetData(Vertices.data())
        .SetStreamIndex(0)
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0})
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 1});
    gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
    VertexBufferCreateInfo.Init().AddVertexStream(VertexStream1);
    Result->VertexBuffer = Context->CreateVertexBuffer(VertexBufferCreateInfo);

    Result->IndexBuffer = Context->CreateBuffer(Indices.size() * sizeof(u32), gfx::bufferUsage::IndexBuffer, gfx::memoryUsage::GpuOnly);
    Context->CopyDataToBuffer(Result->IndexBuffer, Indices.data(), Indices.size() * sizeof(u32), 0);
    
    Result->VertexData = Vertices;
    Result->IndexData = Indices;

    Result->Count = 36;
    Result->Start=0;
    return Result;    
}

std::shared_ptr<indexedGeometryBuffers>  GetSphereGeometry()
{
    gfx::context *Context = gfx::context::Get();
    std::shared_ptr<indexedGeometryBuffers> Result = std::make_shared<indexedGeometryBuffers>();
    
    std::vector<vertex> Vertices;
    std::vector<u32> Indices;
    u32 NumSlices = 32;
    f32 Radius = 1.0f;
    for(int x=0, inx = 0; x<=NumSlices; x++) {
        for(int y=0; y<=NumSlices; y++, inx++) {
            float xAngle = ((float)x / (float)NumSlices) * PI;
            float yAngle = ((float)y / (float)NumSlices) * TWO_PI;
            
            float posx = Radius * std::sin(xAngle) * std::cos(yAngle);
            float posz = Radius * std::sin(xAngle) * std::sin(yAngle);
            float posy = Radius * std::cos(xAngle);

            Vertices.push_back({
                v4f(posx, posy, posz, (float)x / (float) NumSlices),
                v4f(posx, posy, posz, (float)y / (float) NumSlices)
            });

            if(y < NumSlices && x < NumSlices) {
                Indices.push_back(inx + 1);
                Indices.push_back(inx + NumSlices+1);
                Indices.push_back(inx);

                Indices.push_back(inx + NumSlices + 1);
                Indices.push_back(inx + NumSlices);
                Indices.push_back(inx);
            } else if(x < NumSlices){ // If last of the row
                Indices.push_back(inx + 1);
                Indices.push_back(inx - NumSlices);
                Indices.push_back(inx);

                Indices.push_back(inx + 1);
                Indices.push_back(inx + NumSlices);
                Indices.push_back(inx);
            }
        }
    }
    CalculateTangents(Vertices, Indices);

    gfx::vertexStreamData VertexStream1 = {};
    VertexStream1
        .SetSize(Vertices.size() * sizeof(vertex))
        .SetStride(sizeof(vertex))
        .SetData(Vertices.data())
        .SetStreamIndex(0)
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0})
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 1});
    gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
    VertexBufferCreateInfo.Init().AddVertexStream(VertexStream1);
    Result->VertexBuffer = Context->CreateVertexBuffer(VertexBufferCreateInfo);


    Result->IndexBuffer = Context->CreateBuffer(Indices.size() * sizeof(u32), gfx::bufferUsage::IndexBuffer, gfx::memoryUsage::GpuOnly);
    Context->CopyDataToBuffer(Result->IndexBuffer, Indices.data(), Indices.size() * sizeof(u32), 0);
    
    Result->VertexData.resize(sizeof(Vertices));
    memcpy(Result->VertexData.data(), &Vertices, sizeof(Vertices));
    Result->IndexData.resize(sizeof(Indices));
    memcpy(Result->IndexData.data(), &Indices, sizeof(Indices));

    Result->Count = Indices.size();
    Result->Start=0;
    return Result;    
}

std::shared_ptr<indexedGeometryBuffers>  GetConeGeometry()
{
    gfx::context *Context = gfx::context::Get();
    std::shared_ptr<indexedGeometryBuffers> Result = std::make_shared<indexedGeometryBuffers>();
    
    std::vector<vertex> Vertices;
    std::vector<u32> Indices;
    uint32_t NumSlices = 32;
    float Length = 1;
    float Radius = 0.25;

    //Circle center
    Vertices.push_back({
        v4f(0, 0, -Length * 0.5, 0),
        v4f(0, 0, -1, 0)
    });

    //Tip of the cone
    Vertices.push_back({
        v4f(0, 0, Length * 0.5, 0),
        v4f(0, 0,  1, 1)
    });



    for(uint32_t i=0; i<=NumSlices; i++) {
        float inx = ((float)i / (float)NumSlices) * TWO_PI;
        float x = Radius * std::cos(inx);
        float y = Radius * std::sin(inx);

        Vertices.push_back({
            v4f(x, y, -Length * 0.5, (float)i/(float)NumSlices),
            v4f(glm::vec3(0, 0, -1), 0)
        });
        
        //Circular base
        Indices.push_back(i);
        Indices.push_back(0);
        Indices.push_back(i+1);
        
        //Base to tip
        Indices.push_back(i);
        Indices.push_back(1);
        Indices.push_back(i+1);
    }
    
    Indices.push_back(NumSlices);
    Indices.push_back(0);
    Indices.push_back(2);
    
    Indices.push_back(NumSlices);
    Indices.push_back(2);    
    Indices.push_back(1);
    CalculateTangents(Vertices, Indices);
    
    gfx::vertexStreamData VertexStream1 = {};
    VertexStream1
        .SetSize(Vertices.size() * sizeof(vertex))
        .SetStride(sizeof(vertex))
        .SetData(Vertices.data())
        .SetStreamIndex(0)
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0})
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 1});
    gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
    VertexBufferCreateInfo.Init().AddVertexStream(VertexStream1);
    Result->VertexBuffer = Context->CreateVertexBuffer(VertexBufferCreateInfo);


    Result->IndexBuffer = Context->CreateBuffer(Indices.size() * sizeof(u32), gfx::bufferUsage::IndexBuffer, gfx::memoryUsage::GpuOnly);
    Context->CopyDataToBuffer(Result->IndexBuffer, Indices.data(), Indices.size() * sizeof(u32), 0);
    
    Result->VertexData.resize(sizeof(Vertices));
    memcpy(Result->VertexData.data(), &Vertices, sizeof(Vertices));
    Result->IndexData.resize(sizeof(Indices));
    memcpy(Result->IndexData.data(), &Indices, sizeof(Indices));

    Result->Count = Indices.size();
    Result->Start=0;
    return Result;  
}

std::shared_ptr<indexedGeometryBuffers>  GetCapsuleGeometry()
{
    gfx::context *Context = gfx::context::Get();
    std::shared_ptr<indexedGeometryBuffers> Result = std::make_shared<indexedGeometryBuffers>();
    
    std::vector<vertex> Vertices;
    std::vector<u32> Indices;
    s32 NumSlices = 12;
    f32 Height = 4;
    f32 Radius = 1;
    f32 HalfExtents = (Height - (Radius * 2.0)) * 0.5;

    for(int x=0, inx = 0; x<=NumSlices; x++) {
        for(int y=0; y<=NumSlices; y++, inx++) {
            float xAngle = ((float)x / (float)NumSlices) * PI;
            float yAngle = ((float)y / (float)NumSlices) * TWO_PI;
            
            float posx = Radius * std::sin(xAngle) * std::cos(yAngle);
            float posz = Radius * std::sin(xAngle) * std::sin(yAngle);
            float posy = Radius * std::cos(xAngle);
            posy = posy <= 0 ? posy -= HalfExtents : posy += HalfExtents; 

            Vertices.push_back({
                v4f(posx, posy, posz, (float)x / (float) NumSlices),
                v4f(posx, posy, posz, (float)y / (float) NumSlices)
            });

            if(y < NumSlices && x < NumSlices) {
                Indices.push_back(inx + 1);
                Indices.push_back(inx + NumSlices+1);
                Indices.push_back(inx);

                Indices.push_back(inx + NumSlices + 1);
                Indices.push_back(inx + NumSlices);
                Indices.push_back(inx);
            } else if(x < NumSlices){ // If last of the row
                Indices.push_back(inx + 1);
                Indices.push_back(inx - NumSlices);
                Indices.push_back(inx);

                Indices.push_back(inx + 1);
                Indices.push_back(inx + NumSlices);
                Indices.push_back(inx);
            }
        }        
    }    
    CalculateTangents(Vertices, Indices);
    
    gfx::vertexStreamData VertexStream1 = {};
    VertexStream1
        .SetSize(Vertices.size() * sizeof(vertex))
        .SetStride(sizeof(vertex))
        .SetData(Vertices.data())
        .SetStreamIndex(0)
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0})
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 1});
    gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
    VertexBufferCreateInfo.Init().AddVertexStream(VertexStream1);
    Result->VertexBuffer = Context->CreateVertexBuffer(VertexBufferCreateInfo);


    Result->IndexBuffer = Context->CreateBuffer(Indices.size() * sizeof(u32), gfx::bufferUsage::IndexBuffer, gfx::memoryUsage::GpuOnly);
    Context->CopyDataToBuffer(Result->IndexBuffer, Indices.data(), Indices.size() * sizeof(u32), 0);
    
    Result->VertexData.resize(sizeof(Vertices));
    memcpy(Result->VertexData.data(), &Vertices, sizeof(Vertices));
    Result->IndexData.resize(sizeof(Indices));
    memcpy(Result->IndexData.data(), &Indices, sizeof(Indices));

    Result->Count = Indices.size();
    Result->Start=0;
    return Result;  
}


std::shared_ptr<indexedGeometryBuffers>  GetCylinderGeometry()
{
    gfx::context *Context = gfx::context::Get();
    std::shared_ptr<indexedGeometryBuffers> Result = std::make_shared<indexedGeometryBuffers>();
    
    std::vector<vertex> Vertices;
    std::vector<u32> Indices;

    int NumSlices = 12;
    //1. Top  circle
    //_____________________________________________________
    Vertices.push_back({
        v4f(0, 1, 0, 0.5),
        v4f(0, 1, 0, 0.5)
    });

    for(uint32_t i=1; i<NumSlices +1; i++) {
        float inx = ((float)i / (float)NumSlices) * TWO_PI;
        float x = std::cos(inx);
        float y = std::sin(inx);

        Vertices.push_back({
            v4f(x, 1, y, x),
            v4f(x, 1, y, y)
        });
        
        Indices.push_back(0);
        Indices.push_back(i);
        Indices.push_back(i-1);
    }
    
    Indices.push_back(NumSlices);
    Indices.push_back(0);
    Indices.push_back(1);
    //_____________________________________________________
    
    //2. Bottom  circle
    //_____________________________________________________
    Vertices.push_back({
        v4f(0, -1, 0, 0.5),
        v4f(0, -1, 0, 0.5)
    });
    int topCenterInx = Vertices.size() -1;

    for(uint32_t i=1; i<NumSlices +1; i++) {
        float inx = ((float)i / (float)NumSlices) * TWO_PI;
        float x = std::cos(inx);
        float y = std::sin(inx);

        Vertices.push_back({
            v4f(x, -1, y, x),
            v4f(x, -1, y, y)
        });
        
        Indices.push_back(topCenterInx);
        Indices.push_back(topCenterInx + i);
        Indices.push_back(topCenterInx + i-1);
    }
    
    Indices.push_back(topCenterInx + NumSlices);
    Indices.push_back(topCenterInx);
    Indices.push_back(topCenterInx + 1);
    //_____________________________________________________


    //3. Sides
    //_____________________________________________________
    for(int i=1; i<NumSlices+1; i++) {
        Indices.push_back(i+1);
        Indices.push_back(topCenterInx + i);
        Indices.push_back(i);

        Indices.push_back(i + 1);
        Indices.push_back(topCenterInx + i + 1);
        Indices.push_back(topCenterInx + i);
    }
    
    Indices.push_back(1);
    Indices.push_back(NumSlices);
    Indices.push_back(NumSlices * 2 + 1);

    Indices.push_back(1);
    Indices.push_back(NumSlices + 2);
    Indices.push_back(NumSlices * 2 + 1);
    //_____________________________________________________      
    CalculateTangents(Vertices, Indices);
    
    gfx::vertexStreamData VertexStream1 = {};
    VertexStream1
        .SetSize(Vertices.size() * sizeof(vertex))
        .SetStride(sizeof(vertex))
        .SetData(Vertices.data())
        .SetStreamIndex(0)
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0})
        .AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 1});
    gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
    VertexBufferCreateInfo.Init().AddVertexStream(VertexStream1);
    Result->VertexBuffer = Context->CreateVertexBuffer(VertexBufferCreateInfo);


    Result->IndexBuffer = Context->CreateBuffer(Indices.size() * sizeof(u32), gfx::bufferUsage::IndexBuffer, gfx::memoryUsage::GpuOnly);
    Context->CopyDataToBuffer(Result->IndexBuffer, Indices.data(), Indices.size() * sizeof(u32), 0);
    
    Result->VertexData.resize(sizeof(Vertices));
    memcpy(Result->VertexData.data(), &Vertices, sizeof(Vertices));
    Result->IndexData.resize(sizeof(Indices));
    memcpy(Result->IndexData.data(), &Indices, sizeof(Indices));

    Result->Count = Indices.size();
    Result->Start=0;
    return Result;     
}

void indexedGeometryBuffers::Serialize(std::string FileName)
{
    std::ofstream FileStream;
    FileStream.open(FileName, std::ios::trunc | std::ios::binary);
    assert(FileStream.is_open());


    
    u32 NameSize = this->Name.size();
    FileStream.write((char*)&NameSize, sizeof(u32));
    FileStream.write(this->Name.data(), this->Name.size());

    FileStream.write((char*)&Start, sizeof(u32));
    FileStream.write((char*)&Count, sizeof(u32));
    
    FileStream.write((char*)&ID, sizeof(u32));

    
    sz VertexDataSize = this->VertexData.size();
    FileStream.write((char*)&VertexDataSize, sizeof(sz));
    FileStream.write((char*)this->VertexData.data(), VertexDataSize * sizeof(vertex));
    
    sz IndexDataSize = this->IndexData.size();
    FileStream.write((char*)&IndexDataSize, sizeof(sz));
    FileStream.write((char*)this->IndexData.data(), IndexDataSize * sizeof(u32));
}

void indexedGeometryBuffers::Destroy()
{
    gfx::context::Get()->DestroyAccelerationStructure(this->BLAS);
    gfx::context::Get()->QueueDestroyBuffer(this->IndexBuffer);
    gfx::context::Get()->QueueDestroyVertexBuffer(this->VertexBuffer);    
}

std::shared_ptr<indexedGeometryBuffers> indexedGeometryBuffers::Deserialize(const std::string &FileName)
{
    std::shared_ptr<indexedGeometryBuffers> Result = std::make_shared<indexedGeometryBuffers>();

    std::ifstream FileStream;
    FileStream.open(FileName, std::ios::binary);
    assert(FileStream.is_open());

    u32 NameSize;
    FileStream.read((char*)&NameSize, sizeof(u32));
    Result->Name.resize(NameSize);
    FileStream.read(Result->Name.data(), Result->Name.size());

    FileStream.read((char*)&Result->Start, sizeof(u32));
    FileStream.read((char*)&Result->Count, sizeof(u32));
    
    FileStream.read((char*)&Result->ID, sizeof(u32));

    
    sz VertexDataSize;
    FileStream.read((char*)&VertexDataSize, sizeof(sz));
    Result->VertexData.resize(VertexDataSize);
    FileStream.read((char*)Result->VertexData.data(), VertexDataSize * sizeof(vertex));
    
    sz IndexDataSize;
    FileStream.read((char*)&IndexDataSize, sizeof(sz));
    Result->IndexData.resize(IndexDataSize);
    FileStream.read((char*)Result->IndexData.data(), IndexDataSize * sizeof(u32));

    Result->BuildBuffers();
    return Result;
}

}