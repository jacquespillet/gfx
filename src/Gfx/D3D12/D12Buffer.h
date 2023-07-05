#pragma once

#include <wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <d3dx12.h>
namespace gfx
{
struct d3d12BufferData
{
    ComPtr<ID3D12Resource> Handle;
    D3D12_VERTEX_BUFFER_VIEW BufferView;
    D3D12_RESOURCE_STATES ResourceState;
    D3D12_HEAP_TYPE HeapType;
    
    CD3DX12_CPU_DESCRIPTOR_HANDLE CPUHandle;
    CD3DX12_GPU_DESCRIPTOR_HANDLE GPUHandle;

    uint32_t OffsetInHeap = 0;

    void Transition(ID3D12GraphicsCommandList *CommandList, D3D12_RESOURCE_STATES Destination);
};



}