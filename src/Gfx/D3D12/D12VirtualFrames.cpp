#include "D12VirtualFrames.h"

#include "../Include/Context.h"
#include "../Include/Swapchain.h"
#include "D12Context.h"
#include "D12Swapchain.h"
#include "D12Common.h"
#include "D12CommandBuffer.h"
#include <d3dx12.h>

namespace gfx
{

void virtualFramesProvider::Init()
{
    context *Context = context::Get();
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(Context->ApiContextData);
    std::shared_ptr<swapchain> Swapchain = Context->Swapchain;
    GET_API_DATA(D12SwapchainData, d3d12SwapchainData, Swapchain);

    // Create frame resources.
    {
        // Create a RTV for each frame.
        for (UINT n = 0; n < d12Constants::FrameCount; n++)
        {
            
            ThrowIfFailed(D12Data->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&CommandAllocators[n])));
        }
    }

    CommandBuffer = CreateD3D12CommandBuffer(CommandAllocators[D12SwapchainData->GetFrameIndex()]);
    // // Create the command list.


    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        ThrowIfFailed(D12Data->Device->CreateFence(FenceValues[D12SwapchainData->GetFrameIndex()], D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence)));
        FenceValues[D12SwapchainData->GetFrameIndex()]++;

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
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(Context->ApiContextData);
    std::shared_ptr<swapchain> Swapchain = Context->Swapchain;
    GET_API_DATA(D12SwapchainData, d3d12SwapchainData, Swapchain);


    // Schedule a Signal command in the queue.
    ThrowIfFailed(D12Data->CommandQueue->Signal(Fence.Get(), FenceValues[D12SwapchainData->GetFrameIndex()]));

    // Wait until the fence has been processed.
    ThrowIfFailed(Fence->SetEventOnCompletion(FenceValues[D12SwapchainData->GetFrameIndex()], FenceEvent));
    WaitForSingleObjectEx(FenceEvent, INFINITE, FALSE);

    // Increment the fence value for the current frame.
    FenceValues[D12SwapchainData->GetFrameIndex()]++;
}

void virtualFramesProvider::StartFrame()
{
    context *Context = context::Get();
    std::shared_ptr<swapchain> Swapchain = Context->Swapchain;
    GET_API_DATA(D12SwapchainData, d3d12SwapchainData, Swapchain);

    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(CommandAllocators[D12SwapchainData->GetFrameIndex()]->Reset());


    (std::static_pointer_cast<d3d12CommandBufferData>(CommandBuffer->ApiData))->CommandAllocator = CommandAllocators[D12SwapchainData->GetFrameIndex()].Get();
    
    // // However, when ExecuteCommandList() is called on a particular command 
    // // list, that command list can then be reset at any time and must be before 
    // // re-recording.
    // ThrowIfFailed(CommandList->Reset(CommandAllocators[D12SwapchainData->GetFrameIndex()].Get(), m_pipelineState.Get()));     
}

void virtualFramesProvider::EndFrame()
{
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(context::Get()->ApiContextData);
    std::shared_ptr<d3d12CommandBufferData> D12CommandBuffer = std::static_pointer_cast<d3d12CommandBufferData>(CommandBuffer->ApiData);
    std::shared_ptr<d3d12SwapchainData> D12SwapchainData = std::static_pointer_cast<d3d12SwapchainData>(context::Get()->Swapchain->ApiData);

    ThrowIfFailed(D12CommandBuffer->CommandList->Close());
    
    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { D12CommandBuffer->CommandList.Get() };
    D12Data->CommandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    ThrowIfFailed(D12SwapchainData->SwapChain->Present(1, 0));


    // Schedule a Signal command in the queue.
    const UINT64 currentFenceValue = FenceValues[D12SwapchainData->GetFrameIndex()];
    ThrowIfFailed(D12Data->CommandQueue->Signal(Fence.Get(), currentFenceValue));

    // Update the frame index.
    D12SwapchainData->SetFrameIndex(D12SwapchainData->SwapChain->GetCurrentBackBufferIndex());

    // If the next frame is not ready to be rendered yet, wait until it is ready.
    if (Fence->GetCompletedValue() < FenceValues[D12SwapchainData->GetFrameIndex()])
    {
        ThrowIfFailed(Fence->SetEventOnCompletion(FenceValues[D12SwapchainData->GetFrameIndex()], FenceEvent));
        WaitForSingleObjectEx(FenceEvent, INFINITE, FALSE);
    }

    // Set the fence value for the next frame.  
    FenceValues[D12SwapchainData->GetFrameIndex()] = currentFenceValue + 1;
}

}