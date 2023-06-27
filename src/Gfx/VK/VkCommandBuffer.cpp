#if API==VK
#include "../Include/GfxContext.h"
#include "../Include/Buffer.h"
#include "../Include/Swapchain.h"
#include "VkGfxContext.h"
#include "VkSwapchain.h"
#include "VkCommandBuffer.h"
#include "VkBuffer.h"
#include "VkRenderPass.h"
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

void commandBuffer::BeginPass(renderPassHandle RenderPassHandle)
{
    context *Context = context::Get();
    GET_CONTEXT(VkData, Context);

    vkCommandBufferData *VkCommandBufferData = (vkCommandBufferData*)this->ApiData;

    swapchain *Swapchain = Context->Swapchain;
    vkSwapchainData *VkSwapchainData = (vkSwapchainData*)Swapchain->ApiData;
    vk::Framebuffer Framebuffer = VkSwapchainData->Framebuffers[VkSwapchainData->CurrentIndex];

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
                   .setFramebuffer(Framebuffer);

    vk::CommandBuffer CommandBuffer = ((vkCommandBufferData*)this->ApiData)->Handle;
    CommandBuffer.beginRenderPass(RenderPassBegin, vk::SubpassContents::eInline);
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