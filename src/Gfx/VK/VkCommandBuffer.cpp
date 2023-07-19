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
#include "VkUniform.h"
#include "VkImage.h"
namespace gfx
{

std::shared_ptr<commandBuffer> CreateVkCommandBuffer(vk::CommandBuffer VkCommandBuffer)
{
    std::shared_ptr<commandBuffer>  CommandBuffer = std::make_shared<commandBuffer>();
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

void commandBuffer::BeginPass(framebufferHandle FramebufferHandle, clearColorValues ClearColor, clearDepthStencilValues DepthStencil)
{
    context *Context = context::Get();
    GET_CONTEXT(VkData, Context);

    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(this->ApiData);

    framebuffer *Framebuffer = (framebuffer*) Context->ResourceManager.Framebuffers.GetResource(FramebufferHandle);
    std::shared_ptr<vkFramebufferData> VkFramebufferData = std::static_pointer_cast<vkFramebufferData>(Framebuffer->ApiData);
    vk::Framebuffer VkFramebufferHandle = VkFramebufferData->Handle;

    renderPass *RenderPass = (renderPass*) Context->ResourceManager.RenderPasses.GetResource(Framebuffer->RenderPass);
    std::shared_ptr<vkRenderPassData> VkRenderPassData = std::static_pointer_cast<vkRenderPassData>(RenderPass->ApiData);
    vk::RenderPass VkRenderPassHandle = VkRenderPassData->NativeHandle;

    vk::Rect2D RenderArea;
    RenderArea.setExtent(vk::Extent2D(Framebuffer->Width, Framebuffer->Height));
    RenderArea.setOffset(vk::Offset2D(0, 0));

    std::array<float, 4> ClearColorsArray = { ClearColor.R, ClearColor.G, ClearColor.B, ClearColor.A};
    vk::ClearValue ClearValue[3];
    if(VkFramebufferData->IsMultiSampled)
    {
        ClearValue[0].color.setFloat32(ClearColorsArray);
        ClearValue[1].color.setFloat32(ClearColorsArray);
        ClearValue[2].depthStencil.setDepth(DepthStencil.Depth);
        ClearValue[2].depthStencil.setStencil(DepthStencil.Stencil);
    }
    else
    {
        ClearValue[0].color.setFloat32(ClearColorsArray);
        ClearValue[1].depthStencil.setDepth(DepthStencil.Depth);
        ClearValue[1].depthStencil.setStencil(DepthStencil.Stencil);
    }

    vk::RenderPassBeginInfo RenderPassBegin;
    RenderPassBegin.setRenderPass(VkRenderPassHandle)
                   .setClearValues(ClearValue)
                   .setRenderArea(RenderArea)
                   .setFramebuffer(VkFramebufferHandle);

    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.beginRenderPass(RenderPassBegin, vk::SubpassContents::eInline);
    
}

void commandBuffer::BindGraphicsPipeline(pipelineHandle PipelineHandle)
{
    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(this->ApiData);

    pipeline *Pipeline = (pipeline*)context::Get()->ResourceManager.Pipelines.GetResource(PipelineHandle);
    std::shared_ptr<vkPipelineData> VkPipeline = std::static_pointer_cast<vkPipelineData>(Pipeline->ApiData);

    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, VkPipeline->NativeHandle);

    VkCommandBufferData->BoundPipeline = PipelineHandle;
    
}

void commandBuffer::BindComputePipeline(pipelineHandle PipelineHandle)
{
    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(this->ApiData);

    pipeline *Pipeline = (pipeline*)context::Get()->ResourceManager.Pipelines.GetResource(PipelineHandle);
    std::shared_ptr<vkPipelineData> VkPipeline = std::static_pointer_cast<vkPipelineData>(Pipeline->ApiData);

    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, VkPipeline->NativeHandle);

    VkCommandBufferData->BoundPipeline = PipelineHandle;
}

void commandBuffer::Dispatch(u32 NumGroupX, u32 NumGroupY, u32 NumGroupZ)
{
    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(this->ApiData);
    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.dispatch(NumGroupX, NumGroupY, NumGroupZ);
}

void commandBuffer::BindIndexBuffer(bufferHandle BufferHandle, u32 Offset, indexType IndexType)
{
    buffer *Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(BufferHandle);
    std::shared_ptr<vkBufferData> VkBuffer = std::static_pointer_cast<vkBufferData>(Buffer->ApiData);
    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.bindIndexBuffer(VkBuffer->Handle, Offset, IndexTypeToNative(IndexType));
}

void commandBuffer::BindVertexBuffer(vertexBufferHandle BufferHandle)
{
    vertexBuffer *VertexBuffer = (vertexBuffer*)context::Get()->ResourceManager.VertexBuffers.GetResource(BufferHandle);

    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    for(u32 i=0; i<VertexBuffer->NumVertexStreams; i++)
    {
        buffer *Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(VertexBuffer->VertexStreams[i].Buffer);
        std::shared_ptr<vkBufferData> VkBuffer = std::static_pointer_cast<vkBufferData>(Buffer->ApiData);

        u64 Offsets[] = {0};
        CommandBuffer.bindVertexBuffers(VertexBuffer->VertexStreams[i].StreamIndex, 1, &VkBuffer->Handle, Offsets);
    }
}

void commandBuffer::SetViewport(f32 X, f32 Y, f32 Width, f32 Height)
{
    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    vk::Viewport Viewport(X, Height, Width, -Height, 0, 1);
    CommandBuffer.setViewport(0, Viewport);
}

void commandBuffer::SetScissor(s32 OffsetX, s32 OffsetY, u32 Width, u32 Height)
{
    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    vk::Rect2D Scissor({OffsetX,OffsetY}, {Width, Height});
    CommandBuffer.setScissor(0, Scissor);
}


void commandBuffer::DrawArrays(u32 Start, u32 Count, u32 InstanceCount)
{
    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.draw(Count, InstanceCount, Start, 0);
}

void commandBuffer::DrawIndexed(u32 Start, u32 Count, u32 InstanceCount)
{
    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.drawIndexed(Count, InstanceCount, Start, 0, 0);
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

void commandBuffer::CopyBufferToImage(const bufferInfo &Source, const imageInfo &Destination)
{
    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(this->ApiData);
    std::shared_ptr<vkImageData> VKImage = std::static_pointer_cast<vkImageData>(Destination.Resource->ApiData);
    std::shared_ptr<vkBufferData> VKBuffer = std::static_pointer_cast<vkBufferData>(Source.Resource->ApiData);

    if(Destination.Usage != imageUsage::TRANSFER_DESTINATION)
    {

        auto DestinationRange = GetDefaultImageSubresourceRange(*Destination.Resource);
        vk::ImageMemoryBarrier ToTransferDestBarrier;
        ToTransferDestBarrier.setSrcAccessMask(ImageUsageToAccessFlags(Destination.Usage))
                             .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                             .setOldLayout(ImageUsageToImageLayout(Destination.Usage))
                             .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                             .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                             .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                             .setImage(VKImage->Handle)
                             .setSubresourceRange(DestinationRange);

        VkCommandBufferData->Handle.pipelineBarrier(
            ImageUsageToPipelineStage(Destination.Usage),
            vk::PipelineStageFlagBits::eTransfer,
            {},
            {},
            {},
            ToTransferDestBarrier
        );
    }

    auto DestinationLayers = GetDefaultImageSubresourceLayers(*Destination.Resource, Destination.MipLevel, Destination.Layer);
    vk::BufferImageCopy BufferToImageCopyInfo;
    BufferToImageCopyInfo.setBufferOffset(Source.Offset)
                         .setBufferImageHeight(0)
                         .setBufferRowLength(0)
                         .setImageSubresource(DestinationLayers)
                         .setImageOffset(vk::Offset3D(0,0,0))
                         .setImageExtent(vk::Extent3D(
                            Destination.Resource->GetMipLevelWidth(Destination.MipLevel),
                            Destination.Resource->GetMipLevelHeight(Destination.MipLevel),
                            1
                         ));
    VkCommandBufferData->Handle.copyBufferToImage(
        VKBuffer->Handle,
        VKImage->Handle,
        vk::ImageLayout::eTransferDstOptimal,
        BufferToImageCopyInfo
    );
}

void commandBuffer::TransferLayout(const image &Texture, imageUsage::bits OldLayout, imageUsage::bits NewLayout)
{
    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(this->ApiData);
    std::shared_ptr<vkImageData> VKImage = std::static_pointer_cast<vkImageData>(Texture.ApiData);

    auto Barrier = GetImageMemoryBarrier(Texture, OldLayout, NewLayout);
    VkCommandBufferData->Handle.pipelineBarrier(
        ImageUsageToPipelineStage(OldLayout),
        ImageUsageToPipelineStage(NewLayout),
        {},
        {},
        {},
        Barrier
    );    
}





void commandBuffer::BindUniformGroup(std::shared_ptr<uniformGroup> Group, u32 Binding)
{
    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(this->ApiData);
    pipeline *Pipeline = (pipeline*)context::Get()->ResourceManager.Pipelines.GetResource(VkCommandBufferData->BoundPipeline);
    std::shared_ptr<vkPipelineData> VkPipeline = std::static_pointer_cast<vkPipelineData>(Pipeline->ApiData);
    std::shared_ptr<vkUniformData> VkUniformData = std::static_pointer_cast<vkUniformData>(Group->ApiData);
    VkCommandBufferData->Handle.bindDescriptorSets(VkPipeline->BindPoint, VkPipeline->PipelineLayout, Binding, 1, &VkUniformData->DescriptorInfos[VkCommandBufferData->BoundPipeline].DescriptorSet, 0, 0);
}



}

#endif