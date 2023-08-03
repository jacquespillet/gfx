#pragma once
#include "../Include/Buffer.h"

#include <wrl.h>
using namespace Microsoft::WRL;

namespace gfx
{

struct d3d11Buffer
{
    ComPtr<ID3D11Buffer> Handle;
    ComPtr<ID3D11ShaderResourceView> StructuredHandle;
    ComPtr<ID3D11UnorderedAccessView> UAVHandle;
    D3D11_MAPPED_SUBRESOURCE MappedSubresource;
};

struct d3d11VertexBuffer
{
    u32 VertexBufferCount = 0;
    bufferHandle VertexBuffers[commonConstants::MaxVertexStreams];
};

}