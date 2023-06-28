#if GFX_API==GFX_VK
#include "../Include/GfxContext.h"
#include "../Include/Buffer.h"
#include "../Include/Swapchain.h"
#include "../Include/Framebuffer.h"
#include "../Include/Memory.h"
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
    commandBuffer *CommandBuffer = (commandBuffer*)AllocateMemory(sizeof(commandBuffer));
    CommandBuffer->ApiData = std::make_shared<vkCommandBufferData>();
    std::shared_ptr<vkCommandBufferData> VkData = std::static_pointer_cast<vkCommandBufferData>(CommandBuffer->ApiData);
    VkData->Handle = VkCommandBuffer;


    return CommandBuffer;
}

void commandBuffer::Begin()
{
    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(this->ApiData);
    
    vk::CommandBufferBeginInfo CommandBufferBeginInfo;
    CommandBufferBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    VkCommandBufferData->Handle.begin(CommandBufferBeginInfo);
}

void commandBuffer::BeginPass(renderPassHandle RenderPassHandle, framebufferHandle FramebufferHandle)
{
    context *Context = context::Get();
    GET_CONTEXT(VkData, Context);

    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(this->ApiData);

    framebuffer *Framebuffer = (framebuffer*) Context->ResourceManager.Framebuffers.GetResource(FramebufferHandle);
    std::shared_ptr<vkFramebufferData> VkFramebufferData = std::static_pointer_cast<vkFramebufferData>(Framebuffer->ApiData);
    vk::Framebuffer VkFramebufferHandle = VkFramebufferData->Handle;

    renderPass *RenderPass = (renderPass*) Context->ResourceManager.RenderPasses.GetResource(RenderPassHandle);
    std::shared_ptr<vkRenderPassData> VkRenderPassData = std::static_pointer_cast<vkRenderPassData>(RenderPass->ApiData);
    vk::RenderPass VkRenderPassHandle = VkRenderPassData->NativeHandle;

    vk::Rect2D RenderArea;
    RenderArea.setExtent(vk::Extent2D(VkData->SurfaceExtent.width, VkData->SurfaceExtent.height));
    RenderArea.setOffset(vk::Offset2D(0, 0));

    vk::RenderPassBeginInfo RenderPassBegin;
    RenderPassBegin.setRenderPass(VkRenderPassHandle)
                   .setClearValues(VkCommandBufferData->Clears)
                   .setRenderArea(RenderArea)
                   .setFramebuffer(VkFramebufferHandle);

    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.beginRenderPass(RenderPassBegin, vk::SubpassContents::eInline);
}

void commandBuffer::BindGraphicsPipeline(pipelineHandle PipelineHandle)
{
    pipeline *Pipeline = (pipeline*)context::Get()->ResourceManager.Pipelines.GetResource(PipelineHandle);
    std::shared_ptr<vkPipelineData> VkPipeline = std::static_pointer_cast<vkPipelineData>(Pipeline->ApiData);

    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, VkPipeline->NativeHandle);
}

void commandBuffer::BindVertexBuffer(bufferHandle BufferHandle)
{
    buffer *Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(BufferHandle);
    std::shared_ptr<vkBufferData> VkBuffer = std::static_pointer_cast<vkBufferData>(Buffer->ApiData);

    u64 Offsets[] = {0};

    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.bindVertexBuffers(0, 1, &VkBuffer->Handle, Offsets);
}

void commandBuffer::SetViewport(f32 X, f32 Y, f32 Width, f32 Height)
{
    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    vk::Viewport Viewport(X, Y, Width, Height, 0, 1);
    CommandBuffer.setViewport(0, Viewport);
}

void commandBuffer::SetScissor(s32 OffsetX, s32 OffsetY, u32 Width, u32 Height)
{
    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    vk::Rect2D Scissor({OffsetX,OffsetY}, {Width, Height});
    CommandBuffer.setScissor(0, Scissor);
}


void commandBuffer::DrawTriangles(uint32_t Start, uint32_t Count)
{
    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.draw(Count, 1, Start, 0);
}

void commandBuffer::EndPass()
{
    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.endRenderPass();
}

void commandBuffer::End()
{
    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(this->ApiData);
    VkCommandBufferData->Handle.end();
}


void commandBuffer::CopyBuffer(const bufferInfo &Source, const bufferInfo &Destination, size_t ByteSize)
{
    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(this->ApiData);
    
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
    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(this->ApiData);
    std::array<float, 4> ClearColors = { R, G, B, A};
    VkCommandBufferData->Clears[0].color = vk::ClearColorValue{ClearColors}; 
}


void commandBuffer::ClearDepthStencil(f32 Depth, f32 Stencil)
{
    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(this->ApiData);
    VkCommandBufferData->Clears[1].depthStencil.depth = Depth;
    VkCommandBufferData->Clears[1].depthStencil.stencil = Stencil;
}


}

#endif