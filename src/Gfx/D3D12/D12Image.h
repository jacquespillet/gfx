#pragma once

#include "../Include/Image.h"
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
    CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle;
    

    D3D12_RESOURCE_STATES ResourceState;
};
}