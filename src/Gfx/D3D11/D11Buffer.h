#pragma once
#include "../Include/Buffer.h"

namespace gfx
{

struct d3d11Buffer
{
    ID3D11Buffer* Handle;
    D3D11_MAPPED_SUBRESOURCE MappedSubresource;
};

struct d3d11VertexBuffer
{
    ID3D11Buffer* Handle;
};

}