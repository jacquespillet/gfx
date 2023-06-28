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
    D3D12_RESOURCE_STATES ResourceState;
    D3D12_HEAP_TYPE HeapType;

    void Transition(ID3D12GraphicsCommandList *CommandList, D3D12_RESOURCE_STATES Destination);
};



}