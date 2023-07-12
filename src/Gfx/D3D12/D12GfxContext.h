#pragma once
#include "../Include/Buffer.h"

#include <wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <dxgi1_6.h>
#include "D12VirtualFrames.h"

namespace gfx
{

struct d3d12Data
{
    ComPtr<IDXGIFactory4> Factory;
    ComPtr<ID3D12Device> Device;
    ComPtr<ID3D12CommandQueue> CommandQueue;
    virtualFramesProvider VirtualFrames;

    std::shared_ptr<commandBuffer> ImmediateCommandBuffer;
    ComPtr<ID3D12CommandAllocator> ImmediateCommandAllocator;
    
    ComPtr<ID3D12DescriptorHeap> CommonDescriptorHeap;
    ComPtr<ID3D12DescriptorHeap> SrvDescriptorHeap;
    u32 DescriptorSize=0;

    stageBuffer StageBuffer;

    uint32_t CurrentHeapOffset=0;

    ComPtr<ID3D12Fence> ImmediateFence;
    HANDLE ImmediateFenceEvent;

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUDescriptorAt(sz Index);
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUDescriptorAt(sz Index);
};

}