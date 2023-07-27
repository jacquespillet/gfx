#pragma once

#include <d3d12.h>
#include <wrl.h>
#include <d3dx12.h>
using namespace Microsoft::WRL;

namespace gfx
{
struct d3d12ImageData
{
    ComPtr<ID3D12Resource> Handle;
    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHandle;
    uint32_t OffsetInHeap = 0;
    

    D3D12_RESOURCE_STATES ResourceState;
};
}