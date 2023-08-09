#include "Include/Geometry.h"
#include "Include/Context.h"
#include <fstream>

#define PI 3.1415926
#define TWO_PI 6.283185

namespace hlgfx
{
indexedGeometryBuffers::indexedGeometryBuffers()
{
    this->UUID = context::Get()->GetUUID();
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


std::shared_ptr<indexedGeometryBuffers>  GetQuadGeometry()
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



std::shared_ptr<indexedGeometryBuffers>  GetCubeGeometry()
{

    gfx::context *Context = gfx::context::Get();
    std::shared_ptr<indexedGeometryBuffers> Result = std::make_shared<indexedGeometryBuffers>();

    vertex Vertices[] =
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
        0,  1,  2,  3,  0,  2,
        4,  5,  6,  7,  4,  6,
        8,  9, 10,  11, 8, 10,
        12, 13, 14, 15, 12, 14,
        16, 17, 18, 19, 16, 18,
        20, 21, 22, 23, 20, 22,
    };
    Result->IndexBuffer = Context->CreateBuffer(sizeof(Indices), gfx::bufferUsage::IndexBuffer, gfx::memoryUsage::GpuOnly);
    Context->CopyDataToBuffer(Result->IndexBuffer, &Indices, sizeof(Indices), 0);
    
    Result->VertexData.resize(sizeof(Vertices));
    memcpy(Result->VertexData.data(), &Vertices, sizeof(Vertices));
    Result->IndexData.resize(sizeof(Indices));
    memcpy(Result->IndexData.data(), &Indices, sizeof(Indices));

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
        
        Indices.push_back(i);
        Indices.push_back(0);
        Indices.push_back(i+1);
        
        Indices.push_back(i);
        Indices.push_back(1);
        Indices.push_back(i-1);
    }
    
    Indices.push_back(NumSlices);
    Indices.push_back(0);
    Indices.push_back(2);
    
    Indices.push_back(NumSlices);
    Indices.push_back(2);    
    Indices.push_back(1);
    
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
    
    u32 UUIDSize = this->UUID.size();
    FileStream.write((char*)&UUIDSize, sizeof(u32));
    FileStream.write(this->UUID.data(), this->UUID.size());

    
    sz VertexDataSize = this->VertexData.size();
    FileStream.write((char*)&VertexDataSize, sizeof(sz));
    FileStream.write((char*)this->VertexData.data(), VertexDataSize * sizeof(vertex));
    
    sz IndexDataSize = this->IndexData.size();
    FileStream.write((char*)&IndexDataSize, sizeof(sz));
    FileStream.write((char*)this->IndexData.data(), IndexDataSize * sizeof(u32));
}

void indexedGeometryBuffers::Destroy()
{
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
    
    u32 UUIDSize = Result->UUID.size();
    FileStream.read((char*)&UUIDSize, sizeof(u32));
    Result->UUID.resize(UUIDSize);
    FileStream.read(Result->UUID.data(), Result->UUID.size());

    
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