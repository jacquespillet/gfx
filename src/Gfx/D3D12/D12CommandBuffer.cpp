#include "D12CommandBuffer.h"
#include "../Include/GfxContext.h"
#include "../Include/Swapchain.h"
#include "../Include/CommandBuffer.h"
#include "../Include/Framebuffer.h"
#include "../Include/Buffer.h"
#include "../Include/Pipeline.h"
#include "D12Pipeline.h"
#include "D12Buffer.h"
#include "D12Image.h"
#include "D12Framebuffer.h"
#include "D12Mapping.h"

#include <d3dx12.h>

namespace gfx
{
    
std::shared_ptr<commandBuffer> CreateD3D12CommandBuffer(ComPtr<ID3D12CommandAllocator> CommandAllocator)
{
    std::shared_ptr<commandBuffer> CommandBuffer = std::make_shared<commandBuffer>();
    CommandBuffer->ApiData = std::make_shared<d3d12CommandBufferData>();
    std::shared_ptr<d3d12CommandBufferData> D12CommandBufferData = std::static_pointer_cast<d3d12CommandBufferData>(CommandBuffer->ApiData);
    D12CommandBufferData->CommandAllocator = CommandAllocator.Get();
    
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(context::Get()->ApiContextData);

    ThrowIfFailed(D12Data->Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, D12CommandBufferData->CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&D12CommandBufferData->CommandList)));
    ThrowIfFailed(D12CommandBufferData->CommandList->Close());

    return CommandBuffer;
}

void commandBuffer::Begin()
{
    std::shared_ptr<d3d12CommandBufferData> D12CommandBufferData = std::static_pointer_cast<d3d12CommandBufferData>(this->ApiData);
    ThrowIfFailed(D12CommandBufferData->CommandList->Reset(D12CommandBufferData->CommandAllocator.Get(), nullptr));

    GET_CONTEXT(D12Data, context::Get());
    ID3D12DescriptorHeap* ppHeaps[] = { D12Data->CommonDescriptorHeap.Get() };
    D12CommandBufferData->CommandList->SetDescriptorHeaps(1, ppHeaps);
}


ID3D12Resource *GetBufferHandle(buffer *Buffer)
{
    std::shared_ptr<d3d12BufferData> D12BufferData = std::static_pointer_cast<d3d12BufferData>(Buffer->ApiData);
    return D12BufferData->Handle.Get();
}

ID3D12Resource *GetImageHandle(image *Image)
{
    std::shared_ptr<d3d12ImageData> D12ImageData = std::static_pointer_cast<d3d12ImageData>(Image->ApiData);
    return D12ImageData->Handle.Get();
}
void commandBuffer::CopyBuffer(const bufferInfo &Source, const bufferInfo &Destination, size_t ByteSize)
{
    std::shared_ptr<d3d12CommandBufferData> D12CommandBufferData = std::static_pointer_cast<d3d12CommandBufferData>(this->ApiData);
    
    assert(Source.Resource->Size >= Source.Offset + ByteSize);
    assert(Destination.Resource->Size >= Destination.Offset + ByteSize);

    ID3D12Resource *SourceHandle = GetBufferHandle(Source.Resource);
    ID3D12Resource *DestHandle = GetBufferHandle(Destination.Resource);

    std::shared_ptr<d3d12BufferData> D12SourceHandle = std::static_pointer_cast<d3d12BufferData>(Source.Resource->ApiData);
    std::shared_ptr<d3d12BufferData> D12DestinationHandle = std::static_pointer_cast<d3d12BufferData>(Destination.Resource->ApiData);
    
    D3D12_RESOURCE_STATES SourceInitialState = D12SourceHandle->ResourceState;
    D3D12_RESOURCE_STATES DestinationInitialState = D12DestinationHandle->ResourceState;

    b8 TransitionSource = (SourceInitialState != D3D12_RESOURCE_STATE_COPY_SOURCE) && (D12SourceHandle->HeapType != D3D12_HEAP_TYPE_READBACK) && (D12SourceHandle->HeapType != D3D12_HEAP_TYPE_UPLOAD);
    b8 TransitionDestination = (DestinationInitialState != D3D12_RESOURCE_STATE_COPY_DEST) && (D12DestinationHandle->HeapType != D3D12_HEAP_TYPE_READBACK) && (D12DestinationHandle->HeapType != D3D12_HEAP_TYPE_UPLOAD);

    if(TransitionSource) D12SourceHandle->Transition(D12CommandBufferData->CommandList.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
    if(TransitionDestination) D12DestinationHandle->Transition(D12CommandBufferData->CommandList.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
    // Copy the contents of the source buffer to the destination buffer
    D12CommandBufferData->CommandList->CopyBufferRegion(DestHandle, Destination.Offset, SourceHandle, Source.Offset, ByteSize);

    // Create a resource barrier to transition the destination buffer back to its original state
    if(TransitionSource) D12SourceHandle->Transition(D12CommandBufferData->CommandList.Get(), SourceInitialState);
    if(TransitionDestination) D12DestinationHandle->Transition(D12CommandBufferData->CommandList.Get(), DestinationInitialState);
}

void commandBuffer::CopyBufferToImage(const bufferInfo &Source, const imageInfo &Destination)
{
    std::shared_ptr<d3d12CommandBufferData> D12CommandBufferData = std::static_pointer_cast<d3d12CommandBufferData>(this->ApiData);
    
    ID3D12Resource *SourceHandle = GetBufferHandle(Source.Resource);
    ID3D12Resource *DestHandle = GetImageHandle(Destination.Resource);

    std::shared_ptr<d3d12BufferData> D12SourceHandle = std::static_pointer_cast<d3d12BufferData>(Source.Resource->ApiData);
    std::shared_ptr<d3d12BufferData> D12DestinationHandle = std::static_pointer_cast<d3d12BufferData>(Destination.Resource->ApiData);

    // D12CommandBufferData->CommandList->CopyTextureRegion(DestHandle, 0, SourceHandle, 0, Destination.Resource->ByteSize);
    
    D3D12_SUBRESOURCE_FOOTPRINT pitchedDesc = { };
    pitchedDesc.Format = FormatToNative(Destination.Resource->Format);
    pitchedDesc.Width = Destination.Resource->Extent.Width;
    pitchedDesc.Height = Destination.Resource->Extent.Height;
    pitchedDesc.Depth = 1;
    pitchedDesc.RowPitch = Destination.Resource->ByteSize / Destination.Resource->Extent.Height;
    
    D3D12_PLACED_SUBRESOURCE_FOOTPRINT placedTexture2D = { 0 };
    placedTexture2D.Offset = Source.Offset;
    placedTexture2D.Footprint = pitchedDesc;


    D12CommandBufferData->CommandList->CopyTextureRegion( 
        &CD3DX12_TEXTURE_COPY_LOCATION( DestHandle, 0 ), 
        0, 0, 0, 
        &CD3DX12_TEXTURE_COPY_LOCATION( SourceHandle, placedTexture2D ), 
        nullptr );
    
}

void commandBuffer::TransferLayout(const image &Texture, imageUsage::bits OldLayout, imageUsage::bits NewLayout)
{
    std::shared_ptr<d3d12CommandBufferData> D12CommandBufferData = std::static_pointer_cast<d3d12CommandBufferData>(this->ApiData);

    std::shared_ptr<d3d12ImageData> D12ImageData = std::static_pointer_cast<d3d12ImageData>(Texture.ApiData);
    CD3DX12_RESOURCE_BARRIER barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(D12ImageData->Handle.Get(),
        ImageUsageToResourceState(OldLayout), ImageUsageToResourceState(NewLayout));
    D12CommandBufferData->CommandList->ResourceBarrier(1, &barrier1);
}

void commandBuffer::BeginPass(framebufferHandle FramebufferHandle, clearColorValues ClearColor, clearDepthStencilValues DepthStencil)
{
    std::shared_ptr<d3d12CommandBufferData> D12CommandBufferData = std::static_pointer_cast<d3d12CommandBufferData>(this->ApiData);

    framebuffer *Framebuffer = (framebuffer*) context::Get()->ResourceManager.Framebuffers.GetResource(FramebufferHandle);
    std::shared_ptr<d3d12FramebufferData> D12FramebufferData = std::static_pointer_cast<d3d12FramebufferData>(Framebuffer->ApiData);
    D12CommandBufferData->CurrentFramebuffer = Framebuffer;

    if(D12FramebufferData->IsSwapchain)
        D12CommandBufferData->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(D12FramebufferData->RenderTargets[D12FramebufferData->CurrentTarget].Get() , D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
    
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(D12FramebufferData->RenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart(), D12FramebufferData->CurrentTarget, D12FramebufferData->RTVDescriptorSize);
    
    D12CommandBufferData->CommandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &D12FramebufferData->DepthBufferViewHeap->GetCPUDescriptorHandleForHeapStart());    
    D12CommandBufferData->CommandList->ClearRenderTargetView(rtvHandle, (f32*)&ClearColor, 0, nullptr);    
    D12CommandBufferData->CommandList->ClearDepthStencilView(D12FramebufferData->DepthBufferViewHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, DepthStencil.Depth, (u8)DepthStencil.Stencil, 0, nullptr);

    
}

void commandBuffer::EndPass()
{
    
    std::shared_ptr<d3d12CommandBufferData> D12CommandBufferData = std::static_pointer_cast<d3d12CommandBufferData>(ApiData);
    std::shared_ptr<d3d12FramebufferData> D12FramebufferData = std::static_pointer_cast<d3d12FramebufferData>(D12CommandBufferData->CurrentFramebuffer->ApiData);

    // Indicate that the back buffer will now be used to present.
    if(D12FramebufferData->IsSwapchain)
        D12CommandBufferData->CommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(D12FramebufferData->RenderTargets[D12FramebufferData->CurrentTarget].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}

void commandBuffer::BindGraphicsPipeline(pipelineHandle PipelineHandle)
{
    std::shared_ptr<d3d12CommandBufferData> D12CommandBufferData = std::static_pointer_cast<d3d12CommandBufferData>(ApiData);
    
    pipeline *Pipeline = (pipeline*)context::Get()->ResourceManager.Pipelines.GetResource(PipelineHandle);
    std::shared_ptr<d3d12PipelineData> D12PipelineData = std::static_pointer_cast<d3d12PipelineData>(Pipeline->ApiData);


    D12CommandBufferData->CurrentPipeline = Pipeline;
    // Set necessary state.
    D12CommandBufferData->CommandList->SetGraphicsRootSignature(D12PipelineData->RootSignature.Get());
    D12CommandBufferData->CommandList->SetPipelineState(D12PipelineData->PipelineState.Get());
}

void commandBuffer::BindVertexBuffer(vertexBufferHandle BufferHandle)
{
    std::shared_ptr<d3d12CommandBufferData> D12CommandBufferData = std::static_pointer_cast<d3d12CommandBufferData>(ApiData);
    vertexBuffer *VertexBuffer = (vertexBuffer*)context::Get()->ResourceManager.VertexBuffers.GetResource(BufferHandle);
    
    
    // Record commands.
    D12CommandBufferData->CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    for(int i=0; i<VertexBuffer->NumVertexStreams; i++)
    {
        buffer *Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(VertexBuffer->VertexStreams[i].Buffer);
        std::shared_ptr<d3d12BufferData> D12BufferData = std::static_pointer_cast<d3d12BufferData>(Buffer->ApiData);

        u64 Offsets[] = {0};
        D12CommandBufferData->CommandList->IASetVertexBuffers(VertexBuffer->VertexStreams[i].StreamIndex, 1, &D12BufferData->BufferView);
    }
}

void commandBuffer::SetViewport(f32 X, f32 Y, f32 Width, f32 Height)
{
    std::shared_ptr<d3d12CommandBufferData> D12CommandBufferData = std::static_pointer_cast<d3d12CommandBufferData>(ApiData);
    CD3DX12_VIEWPORT Viewport = CD3DX12_VIEWPORT(0.0f, 0.0f, (float)context::Get()->Swapchain->Width, (float)context::Get()->Swapchain->Height);
    D12CommandBufferData->CommandList->RSSetViewports(1, &Viewport);
}

void commandBuffer::SetScissor(s32 X, s32 Y, u32 Width, u32 Height)
{
    std::shared_ptr<d3d12CommandBufferData> D12CommandBufferData = std::static_pointer_cast<d3d12CommandBufferData>(ApiData);
    CD3DX12_RECT Scissor = CD3DX12_RECT(0, 0, (LONG)context::Get()->Swapchain->Width, (LONG)context::Get()->Swapchain->Height);
    D12CommandBufferData->CommandList->RSSetScissorRects(1, &Scissor);
}

void commandBuffer::DrawTriangles(uint32_t Start, uint32_t Count)
{
    std::shared_ptr<d3d12CommandBufferData> D12CommandBufferData = std::static_pointer_cast<d3d12CommandBufferData>(this->ApiData);
    D12CommandBufferData->CommandList->DrawInstanced(3, 1, 0, 0);
}

void commandBuffer::End()
{
    std::shared_ptr<d3d12CommandBufferData> D12CommandBufferData = std::static_pointer_cast<d3d12CommandBufferData>(this->ApiData);
    ThrowIfFailed(D12CommandBufferData->CommandList->Close());
}

void commandBuffer::BindUniformGroup(std::shared_ptr<uniformGroup> Group, u32 Binding)
{
    std::shared_ptr<d3d12CommandBufferData> D12CommandBufferData = std::static_pointer_cast<d3d12CommandBufferData>(this->ApiData);
    std::shared_ptr<d3d12PipelineData> D12Pipeline = std::static_pointer_cast<d3d12PipelineData>(D12CommandBufferData->CurrentPipeline->ApiData);
    
    GET_CONTEXT(D12Data, context::Get());

    for(sz i=0; i< Group->Uniforms.size(); i++)
    {
        if(Group->Uniforms[i].Type == uniformType::Buffer)
        {
            buffer* BufferData = (buffer*)(Group->Uniforms[i].Resource);
            std::shared_ptr<d3d12BufferData> D12BufferData = std::static_pointer_cast<d3d12BufferData>(BufferData->ApiData);
            
            if(D12Pipeline->UsedRootParams[Group->Uniforms[i].Binding])
            {
                u32 RootParamIndex = D12Pipeline->BindingRootParamMapping[Group->Uniforms[i].Binding];
                D3D12_GPU_VIRTUAL_ADDRESS VirtualAddress = D12BufferData->Handle->GetGPUVirtualAddress();
                D12CommandBufferData->CommandList->SetGraphicsRootConstantBufferView(RootParamIndex, VirtualAddress);
            }
        }
        if(Group->Uniforms[i].Type == uniformType::Texture2d)
        {
            image* ImageData = (image*)(Group->Uniforms[i].Resource);
            std::shared_ptr<d3d12ImageData> D12ImageData = std::static_pointer_cast<d3d12ImageData>(ImageData->ApiData);
            
            if(D12Pipeline->UsedRootParams[Group->Uniforms[i].Binding])
            {
                u32 RootParamIndex = D12Pipeline->BindingRootParamMapping[Group->Uniforms[i].Binding];
                D12CommandBufferData->CommandList->SetGraphicsRootDescriptorTable(RootParamIndex, D12Data->GetGPUDescriptorAt(D12ImageData->OffsetInHeap));
            }
        }
    }
}
    


}