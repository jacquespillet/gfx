#if API==VK
#include "../Include/GfxContext.h"
#include "../Include/Buffer.h"
#include "../Include/Swapchain.h"
#include "../Include/Framebuffer.h"
#include "VkGfxContext.h"
#include "VkSwapchain.h"
#include "VkCommandBuffer.h"
#include "VkBuffer.h"
#include "VkRenderPass.h"
#include "VkPipeline.h"
#include "VkFramebuffer.h"
namespace gfx
{

commandBuffer *CreateVkCommandBuffer(vk::CommandBuffer VkCommandBuffer)
{
    commandBuffer *CommandBuffer = new commandBuffer();
    CommandBuffer->ApiData = new vkCommandBufferData();
    vkCommandBufferData *VkData = (vkCommandBufferData*)CommandBuffer->ApiData;
    VkData->Handle = VkCommandBuffer;


    return CommandBuffer;
}

void commandBuffer::Begin()
{
    vkCommandBufferData *VkCommandBufferData = (vkCommandBufferData*)this->ApiData;
    
    vk::CommandBufferBeginInfo CommandBufferBeginInfo;
    CommandBufferBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    VkCommandBufferData->Handle.begin(CommandBufferBeginInfo);
}

void commandBuffer::BeginPass(renderPassHandle RenderPassHandle, framebufferHandle FramebufferHandle)
{
    context *Context = context::Get();
    GET_CONTEXT(VkData, Context);

    vkCommandBufferData *VkCommandBufferData = (vkCommandBufferData*)this->ApiData;

    framebuffer *Framebuffer = (framebuffer*) Context->ResourceManager.Framebuffers.GetResource(FramebufferHandle);
    vkFramebufferData *VkFramebufferData = (vkFramebufferData*)Framebuffer->ApiData;
    vk::Framebuffer VkFramebufferHandle = VkFramebufferData->Handle;

    renderPass *RenderPass = (renderPass*) Context->ResourceManager.RenderPasses.GetResource(RenderPassHandle);
    vkRenderPassData *VkRenderPassData = (vkRenderPassData*)RenderPass->ApiData;
    vk::RenderPass VkRenderPassHandle = VkRenderPassData->NativeHandle;

    vk::Rect2D RenderArea;
    RenderArea.setExtent(vk::Extent2D(VkData->SurfaceExtent.width, VkData->SurfaceExtent.height));
    RenderArea.setOffset(vk::Offset2D(0, 0));

    vk::RenderPassBeginInfo RenderPassBegin;
    RenderPassBegin.setRenderPass(VkRenderPassHandle)
                   .setClearValues(VkCommandBufferData->Clears)
                   .setRenderArea(RenderArea)
                   .setFramebuffer(VkFramebufferHandle);

    vk::CommandBuffer CommandBuffer = ((vkCommandBufferData*)this->ApiData)->Handle;
    CommandBuffer.beginRenderPass(RenderPassBegin, vk::SubpassContents::eInline);
}

void commandBuffer::BindGraphicsPipeline(pipelineHandle PipelineHandle)
{
    pipeline *Pipeline = (pipeline*)context::Get()->ResourceManager.Pipelines.GetResource(PipelineHandle);
    vkPipelineData *VkPipeline = (vkPipelineData*)Pipeline->ApiData;

    vk::CommandBuffer CommandBuffer = ((vkCommandBufferData*)this->ApiData)->Handle;
    CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, VkPipeline->NativeHandle);
}

void commandBuffer::BindVertexBuffer(bufferHandle BufferHandle)
{
    buffer *Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(BufferHandle);
    vkBufferData *VkBuffer = (vkBufferData*)Buffer->ApiData;

    u64 Offsets[] = {0};

    vk::CommandBuffer CommandBuffer = ((vkCommandBufferData*)this->ApiData)->Handle;
    CommandBuffer.bindVertexBuffers(0, 1, &VkBuffer->Handle, Offsets);
}

void commandBuffer::SetViewport(f32 X, f32 Y, f32 Width, f32 Height)
{
    vk::CommandBuffer CommandBuffer = ((vkCommandBufferData*)this->ApiData)->Handle;
    vk::Viewport Viewport(X, Y, Width, Height, 0, 1);
    CommandBuffer.setViewport(0, Viewport);
}

void commandBuffer::SetScissor(s32 OffsetX, s32 OffsetY, u32 Width, u32 Height)
{
    vk::CommandBuffer CommandBuffer = ((vkCommandBufferData*)this->ApiData)->Handle;
    vk::Rect2D Scissor({OffsetX,OffsetY}, {Width, Height});
    CommandBuffer.setScissor(0, Scissor);
}


void commandBuffer::DrawTriangles(uint32_t Start, uint32_t Count)
{
    vk::CommandBuffer CommandBuffer = ((vkCommandBufferData*)this->ApiData)->Handle;
    CommandBuffer.draw(Count, 1, Start, 0);
}

void commandBuffer::EndPass()
{
    vk::CommandBuffer CommandBuffer = ((vkCommandBufferData*)this->ApiData)->Handle;
    CommandBuffer.endRenderPass();
}

void commandBuffer::End()
{
    vkCommandBufferData *VkCommandBufferData = (vkCommandBufferData*)this->ApiData;
    VkCommandBufferData->Handle.end();
}

void commandBuffer::CopyBuffer(const bufferInfo &Source, const bufferInfo &Destination, size_t ByteSize)
{
    vkCommandBufferData *VkCommandBufferData = (vkCommandBufferData*)this->ApiData;
    
    assert(Source.Resource->Size >= Source.Offset + ByteSize);
    assert(Destination.Resource->Size >= Destination.Offset + ByteSize);

    vk::BufferCopy BufferCopyInfo;
    BufferCopyInfo.setDstOffset(Destination.Offset)
                  .setSize(ByteSize)
                  .setSrcOffset(Source.Offset);
    
    vk::Buffer SourceHandle = GetBufferHandle(Source.Resource);
    vk::Buffer DestHandle = GetBufferHandle(Destination.Resource);

    VkCommandBufferData->Handle.copyBuffer(SourceHandle, DestHandle, BufferCopyInfo);
}

void commandBuffer::ClearColor(f32 R, f32 G,f32 B,f32 A)
{
    vkCommandBufferData *VkCommandBufferData = (vkCommandBufferData*)this->ApiData;
    std::array<float, 4> ClearColors = { R, G, B, A};
    VkCommandBufferData->Clears[0].color = vk::ClearColorValue{ClearColors}; 
}


void commandBuffer::ClearDepthStencil(f32 Depth, f32 Stencil)
{
    vkCommandBufferData *VkCommandBufferData = (vkCommandBufferData*)this->ApiData;
    VkCommandBufferData->Clears[1].depthStencil.depth = Depth;
    VkCommandBufferData->Clears[1].depthStencil.stencil = Stencil;
}


}

#endif