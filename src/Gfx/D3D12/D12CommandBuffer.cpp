#if GFX_API == GFX_D3D12
#include "D12CommandBuffer.h"
#include "../Include/GfxContext.h"
#include "../Include/Swapchain.h"
#include "../Include/CommandBuffer.h"
#include "../Include/Buffer.h"
#include "../Include/Pipeline.h"
#include "D12Pipeline.h"
#include "D12Buffer.h"

#include <d3dx12.h>

namespace gfx
{
    
commandBuffer *CreateD3D12CommandBuffer(ComPtr<ID3D12CommandAllocator> CommandAllocator)
{
    commandBuffer *CommandBuffer = (commandBuffer*)AllocateMemory(sizeof(commandBuffer));
    CommandBuffer->ApiData = (d3d12CommandBufferData*)AllocateMemory(sizeof(d3d12CommandBufferData));
    d3d12CommandBufferData *D12CommandBufferData = (d3d12CommandBufferData*)CommandBuffer->ApiData;
    D12CommandBufferData->CommandAllocator = CommandAllocator.Get();
    
    d3d12Data *D12Data = (d3d12Data*)(context::Get()->ApiContextData);

    ThrowIfFailed(D12Data->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D12CommandBufferData->CommandAllocator, nullptr, IID_PPV_ARGS(&D12CommandBufferData->CommandList)));
    ThrowIfFailed(D12CommandBufferData->CommandList->Close());

    return CommandBuffer;
}

void commandBuffer::Begin()
{   
}



void commandBuffer::BeginPass(renderPassHandle RenderPass, framebufferHandle Framebuffer)
{
    
}
void commandBuffer::EndPass()
{
    
}

void commandBuffer::BindGraphicsPipeline(pipelineHandle PipelineHandle)
{
    d3d12CommandBufferData *D12CommandBufferData = (d3d12CommandBufferData*)ApiData;
    
    pipeline *Pipeline = (pipeline*)context::Get()->ResourceManager.Pipelines.GetResource(PipelineHandle);
    d3d12PipelineData *D12PipelineData = (d3d12PipelineData*)Pipeline->ApiData;

    ThrowIfFailed(D12CommandBufferData->CommandList->Reset(D12CommandBufferData->CommandAllocator, D12PipelineData->PipelineState.Get()));

    CD3DX12_VIEWPORT m_viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, (float)context::Get()->Swapchain->Width, (float)context::Get()->Swapchain->Height);
    CD3DX12_RECT m_scissorRect = CD3DX12_RECT(0, 0, (LONG)context::Get()->Swapchain->Width, (LONG)context::Get()->Swapchain->Height);

    // Set necessary state.
    D12CommandBufferData->CommandList->SetGraphicsRootSignature(D12PipelineData->RootSignature.Get());
    D12CommandBufferData->CommandList->RSSetViewports(1, &m_viewport);
    D12CommandBufferData->CommandList->RSSetScissorRects(1, &m_scissorRect);

    d3d12SwapchainData *D12SwapchainData = (d3d12SwapchainData*)context::Get()->Swapchain->ApiData;
    // Indicate that the back buffer will be used as a render target.
    D12CommandBufferData->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(D12SwapchainData->RenderTargets[D12SwapchainData->FrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(D12SwapchainData->RenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(), D12SwapchainData->FrameIndex, D12SwapchainData->RTVDescriptorSize);
    D12CommandBufferData->CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);    
    
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    D12CommandBufferData->CommandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void commandBuffer::BindVertexBuffer(bufferHandle BufferHandle)
{
    buffer *Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(BufferHandle);
    d3d12BufferData *D12BufferData = (d3d12BufferData*)Buffer->ApiData;

    d3d12CommandBufferData *D12CommandBufferData = (d3d12CommandBufferData*)ApiData;
    
    // Record commands.
    D12CommandBufferData->CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    D12CommandBufferData->CommandList->IASetVertexBuffers(0, 1, &D12BufferData->BufferView);
    D12CommandBufferData->CommandList->DrawInstanced(3, 1, 0, 0);

    d3d12SwapchainData *D12SwapchainData = (d3d12SwapchainData*)context::Get()->Swapchain->ApiData;
    // Indicate that the back buffer will now be used to present.
    D12CommandBufferData->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(D12SwapchainData->RenderTargets[D12SwapchainData->FrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    ThrowIfFailed(D12CommandBufferData->CommandList->Close());
}

void commandBuffer::SetViewport(f32 X, f32 Y, f32 Width, f32 Height)
{

}

void commandBuffer::SetScissor(s32 X, s32 Y, u32 Width, u32 Height)
{

}

void commandBuffer::ClearColor(f32 R, f32 G,f32 B,f32 A)
{

}

void commandBuffer::ClearDepthStencil(f32 Depth, f32 Stencil)
{

}

void commandBuffer::DrawTriangles(uint32_t Start, uint32_t Count)
{

}


    


}

#endif