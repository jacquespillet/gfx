#pragma once

#include <wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>

namespace gfx
{
struct d3d12BufferData
{
    ComPtr<ID3D12Resource> Handle;
    D3D12_VERTEX_BUFFER_VIEW BufferView;
};
}