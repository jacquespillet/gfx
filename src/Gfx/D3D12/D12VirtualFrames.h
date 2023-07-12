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
    ComPtr<ID3D12CommandAllocator> CommandAllocators[d12Constants::FrameCount];
    UINT64 FenceValues[d12Constants::FrameCount];
    ComPtr<ID3D12Fence> Fence;
    HANDLE FenceEvent;

    std::shared_ptr<commandBuffer> CommandBuffer;
    ComPtr<ID3D12GraphicsCommandList> CommandList;
};
}