#pragma once
#include "D12Swapchain.h"

namespace gfx
{
struct commandBuffer;

struct virtualFramesProvider
{
    void Init();
    
    void WaitForPreviousFrame();
    void StartFrame();
    void EndFrame();
    ComPtr<ID3D12CommandAllocator> CommandAllocators[d3d12SwapchainData::FrameCount];
    UINT64 FenceValues[d3d12SwapchainData::FrameCount];
    ComPtr<ID3D12Fence> Fence;
    HANDLE FenceEvent;

    commandBuffer *CommandBuffer;
    ComPtr<ID3D12GraphicsCommandList> CommandList;
};
}