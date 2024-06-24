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
    v4f Tangent; 
};

struct indexedGeometryBuffers
{
    indexedGeometryBuffers();
    gfx::vertexBufferHandle VertexBuffer = gfx::InvalidHandle;
    gfx::bufferHandle IndexBuffer = gfx::InvalidHandle;
    gfx::accelerationStructureHandle BLAS;
	
    u32 Start = 0;
    u32 Count = 0;

    std::vector<vertex> VertexData;
    std::vector<u32> IndexData;

    std::string Name;
    u32 ID;

    void BuildBuffers();

    void Serialize(std::string FileName);
    static std::shared_ptr<indexedGeometryBuffers> Deserialize(const std::string &FileName);

    void Destroy();
};

void CalculateTangents(std::vector<vertex>& Vertices, std::vector<u32> Indices);

std::shared_ptr<indexedGeometryBuffers>  GetQuadGeometry();
std::shared_ptr<indexedGeometryBuffers>  GetCubeGeometry();
std::shared_ptr<indexedGeometryBuffers>  GetSphereGeometry();
std::shared_ptr<indexedGeometryBuffers>  GetConeGeometry();
std::shared_ptr<indexedGeometryBuffers>  GetCapsuleGeometry();
std::shared_ptr<indexedGeometryBuffers>  GetCylinderGeometry();

std::shared_ptr<indexedGeometryBuffers>  CreateGeometryFromBuffers(void *VertexBufferData, sz VertexBufferSize, void *IndexBufferData, sz IndexBufferSize);

}