#if GFX_API == GFX_D3D12
#include "D12VirtualFrames.h"

#include "../Include/GfxContext.h"
#include "../Include/Swapchain.h"
#include "D12GfxContext.h"
#include "D12Swapchain.h"
#include "D12Common.h"
#include "D12CommandBuffer.h"
#include <d3dx12.h>

namespace gfx
{

void virtualFramesProvider::Init()
{
    context *Context = context::Get();
    d3d12Data *D12Data = (d3d12Data*)Context->ApiContextData;
    swapchain *Swapchain = Context->Swapchain;
    d3d12SwapchainData *D12SwapchainData = (d3d12SwapchainData*)Swapchain->ApiData;

    // Create frame resources.
    {
        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(D12SwapchainData->RenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());

        // Create a RTV for each frame.
        for (UINT n = 0; n < d3d12SwapchainData::FrameCount; n++)
        {
            ThrowIfFailed(D12SwapchainData->SwapChain->GetBuffer(n, IID_PPV_ARGS(&D12SwapchainData->RenderTargets[n])));
            D12Data->Device->CreateRenderTargetView(D12SwapchainData->RenderTargets[n].Get(), nullptr, rtvHandle);
            rtvHandle.Offset(1, D12SwapchainData->RTVDescriptorSize);
            ThrowIfFailed(D12Data->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocators[n])));
        }
    }

    CommandBuffer = CreateD3D12CommandBuffer(CommandAllocators[D12SwapchainData->FrameIndex]);
    // // Create the command list.


    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(D12Data->Device->CreateFence(FenceValues[D12SwapchainData->FrameIndex], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
        FenceValues[D12SwapchainData->FrameIndex]++;

        // Create an event handle to use for frame synchronization.
        FenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (FenceEvent == nullptr)
        {
            ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
        }

        // Wait for the command list to execute; we are reusing the same command 
        // list in our main loop but for now, we just want to wait for setup to 
        // complete before continuing.
        WaitForPreviousFrame();
    }    
}

void virtualFramesProvider::WaitForPreviousFrame()
{
    context *Context = context::Get();
    d3d12Data *D12Data = (d3d12Data*)Context->ApiContextData;
    swapchain *Swapchain = Context->Swapchain;
    d3d12SwapchainData *D12SwapchainData = (d3d12SwapchainData*)Swapchain->ApiData;


    // Schedule a Signal command in the queue.
    ThrowIfFailed(D12Data->CommandQueue->Signal(Fence.Get(), FenceValues[D12SwapchainData->FrameIndex]));

    // Wait until the fence has been processed.
    ThrowIfFailed(Fence->SetEventOnCompletion(FenceValues[D12SwapchainData->FrameIndex], FenceEvent));
    WaitForSingleObjectEx(FenceEvent, INFINITE, FALSE);

    // Increment the fence value for the current frame.
    FenceValues[D12SwapchainData->FrameIndex]++;
}

void virtualFramesProvider::StartFrame()
{
    context *Context = context::Get();
    swapchain *Swapchain = Context->Swapchain;
    d3d12SwapchainData *D12SwapchainData = (d3d12SwapchainData*)Swapchain->ApiData;

    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(CommandAllocators[D12SwapchainData->FrameIndex]->Reset());


    ((d3d12CommandBufferData*)CommandBuffer->ApiData)->CommandAllocator = CommandAllocators[D12SwapchainData->FrameIndex].Get();
    
    // // However, when ExecuteCommandList() is called on a particular command 
    // // list, that command list can then be reset at any time and must be before 
    // // re-recording.
    // ThrowIfFailed(CommandList->Reset(CommandAllocators[D12SwapchainData->FrameIndex].Get(), m_pipelineState.Get()));    
}

void virtualFramesProvider::EndFrame()
{
    d3d12Data *D12Data = (d3d12Data*)context::Get()->ApiContextData;
    d3d12CommandBufferData *D12CommandBuffer = (d3d12CommandBufferData*)CommandBuffer->ApiData;
    d3d12SwapchainData *D12SwapchainData = (d3d12SwapchainData *)context::Get()->Swapchain->ApiData;

    ThrowIfFailed(D12CommandBuffer->CommandList->Close());
    
    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { D12CommandBuffer->CommandList };
    D12Data->CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(D12SwapchainData->SwapChain->Present(1, 0));


    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = FenceValues[D12SwapchainData->FrameIndex];
    ThrowIfFailed(D12Data->CommandQueue->Signal(Fence.Get(), currentFenceValue));

    // Update the frame index.
    D12SwapchainData->FrameIndex = D12SwapchainData->SwapChain->GetCurrentBackBufferIndex();

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (Fence->GetCompletedValue() < FenceValues[D12SwapchainData->FrameIndex])
    {
        ThrowIfFailed(Fence->SetEventOnCompletion(FenceValues[D12SwapchainData->FrameIndex], FenceEvent));
        WaitForSingleObjectEx(FenceEvent, INFINITE, FALSE);
    }

    // Set the fence value for the next frame.  
    FenceValues[D12SwapchainData->FrameIndex] = currentFenceValue + 1;
}

}

#endif