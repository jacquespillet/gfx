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
    D3D12_VERTEX_BUFFER_VIEW VertexBufferView;
    D3D12_INDEX_BUFFER_VIEW IndexBufferView;
    D3D12_RESOURCE_STATES ResourceState;
    D3D12_HEAP_TYPE HeapType;
    
    uint32_t OffsetInHeap = 0;

    void Transition(ID3D12GraphicsCommandList *CommandList, D3D12_RESOURCE_STATES Destination);
};



}