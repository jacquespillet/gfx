#if GFX_API==GFX_VK
#include "../Include/Context.h"
#include "../Include/Buffer.h"
#include "../Include/Swapchain.h"
#include "../Include/Framebuffer.h"
#include "../Include/Memory.h"
#include "VkContext.h"
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
    GET_API_DATA(VkData, vkCommandBufferData, CommandBuffer);
    VkData->Handle = VkCommandBuffer;


    return CommandBuffer;
}

void commandBuffer::Begin()
{
    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, this);
    
    vk::CommandBufferBeginInfo CommandBufferBeginInfo;
    CommandBufferBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    VkCommandBufferData->Handle.begin(CommandBufferBeginInfo);
}

void commandBuffer::BeginPass(framebufferHandle FramebufferHandle, clearColorValues ClearColor, clearDepthStencilValues DepthStencil)
{
    context *Context = context::Get();
    GET_CONTEXT(VkData, Context);

    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, this);

    framebuffer *Framebuffer = Context->GetFramebuffer(FramebufferHandle);
    GET_API_DATA(VkFramebufferData, vkFramebufferData, Framebuffer);
    vk::Framebuffer VkFramebufferHandle = VkFramebufferData->Handle;

    renderPass *RenderPass = Context->GetRenderPass(Framebuffer->RenderPass);
    GET_API_DATA(VkRenderPassData, vkRenderPassData, RenderPass);
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
    
    for (size_t i = 0; i < VkFramebufferData->ColorImagesCount; i++)
    {
        GET_API_DATA(VkImage, vkImageData, VkFramebufferData->ColorImages[i]);
        VkImage->CurrentLayout = RenderPass->Output.ColorFinalLayouts[i];
    }
    GET_API_DATA(VkImage, vkImageData, VkFramebufferData->DepthStencilImage);
    VkImage->CurrentLayout = RenderPass->Output.DepthStencilFinalLayout; 
}

void commandBuffer::BindGraphicsPipeline(pipelineHandle PipelineHandle)
{
    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, this);

    pipeline *Pipeline = context::Get()->GetPipeline(PipelineHandle);
    GET_API_DATA(VkPipeline, vkPipelineData, Pipeline);

    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, VkPipeline->NativeHandle);

    VkCommandBufferData->BoundPipeline = PipelineHandle;
    
}

void commandBuffer::BindComputePipeline(pipelineHandle PipelineHandle)
{
    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, this);

    pipeline *Pipeline = context::Get()->GetPipeline(PipelineHandle);
    GET_API_DATA(VkPipeline, vkPipelineData, Pipeline);

    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, VkPipeline->NativeHandle);

    VkCommandBufferData->BoundPipeline = PipelineHandle;
}

void commandBuffer::Dispatch(u32 NumGroupX, u32 NumGroupY, u32 NumGroupZ)
{
    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, this);
    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.dispatch(NumGroupX, NumGroupY, NumGroupZ);
}

void commandBuffer::BindIndexBuffer(bufferHandle BufferHandle, u32 Offset, indexType IndexType)
{
    buffer *Buffer = context::Get()->GetBuffer(BufferHandle);
    GET_API_DATA(VkBuffer, vkBufferData, Buffer);
    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    CommandBuffer.bindIndexBuffer(VkBuffer->Handle, Offset, IndexTypeToNative(IndexType));
}

void commandBuffer::BindVertexBuffer(vertexBufferHandle BufferHandle)
{
    vertexBuffer *VertexBuffer = context::Get()->GetVertexBuffer(BufferHandle);

    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    for(u32 i=0; i<VertexBuffer->NumVertexStreams; i++)
    {
        buffer *Buffer = context::Get()->GetBuffer(VertexBuffer->VertexStreams[i].Buffer);
        GET_API_DATA(VkBuffer, vkBufferData, Buffer);

        u64 Offsets[] = {0};
        CommandBuffer.bindVertexBuffers(VertexBuffer->VertexStreams[i].StreamIndex, 1, &VkBuffer->Handle, Offsets);
    }
}

void commandBuffer::SetViewport(f32 X, f32 Y, f32 Width, f32 Height, b8 InvertViewport)
{
    vk::CommandBuffer CommandBuffer = (std::static_pointer_cast<vkCommandBufferData>(this->ApiData))->Handle;
    if(InvertViewport)
    {
        vk::Viewport Viewport(X, Height, Width, -Height, 0, 1);
        CommandBuffer.setViewport(0, Viewport);
    }
    else
    {
        vk::Viewport Viewport(X, Y, Width, Height, 0, 1);
        CommandBuffer.setViewport(0, Viewport);
    }
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
    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, this);
    VkCommandBufferData->Handle.end();
}


void commandBuffer::CopyBuffer(const bufferInfo &Source, const bufferInfo &Destination, size_t ByteSize)
{
    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, this);
    
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

void commandBuffer::CopyFramebufferToImage(const framebufferInfo &Source, const imageInfo &Destination)
{
    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, this);
    GET_API_DATA(VKImageDest, vkImageData, Destination.Resource);
    GET_API_DATA(VKFramebuffer, vkFramebufferData, Source.Resource);
    
    image *SrcImage;
    if(Source.Depth) SrcImage = VKFramebuffer->DepthStencilImage.get();
    else SrcImage = VKFramebuffer->ColorImages[Source.Color].get();
    GET_API_DATA(VKImageSrc, vkImageData, SrcImage);

    imageLayout SrcInitialUsage = VKImageSrc->CurrentLayout;
    imageLayout DstInitialUsage = VKImageDest->CurrentLayout;

    if(VKImageDest->CurrentLayout != imageLayout::TransferDstOptimal)
    {
        auto DestinationRange = GetDefaultImageSubresourceRange(*Destination.Resource);
        vk::ImageMemoryBarrier ToTransferDestBarrier;
        ToTransferDestBarrier.setSrcAccessMask(ImageLayoutToAccessFlags(VKImageDest->CurrentLayout))
                             .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                             .setOldLayout(ImageLayoutToNative(VKImageDest->CurrentLayout))
                             .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                             .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                             .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                             .setImage(VKImageDest->Handle)
                             .setSubresourceRange(DestinationRange);

        VkCommandBufferData->Handle.pipelineBarrier(
            ImageLayoutToPipelineStage(VKImageDest->CurrentLayout),
            vk::PipelineStageFlagBits::eTransfer,
            {},
            {},
            {},
            ToTransferDestBarrier
        );
    }

    if(VKImageSrc->CurrentLayout != imageLayout::TransferSrcOptimal)
    {
        auto SourceRange = GetDefaultImageSubresourceRange(*SrcImage);
        vk::ImageMemoryBarrier ToTransferSrcBarrier;
        ToTransferSrcBarrier.setSrcAccessMask(ImageLayoutToAccessFlags(VKImageSrc->CurrentLayout))
                             .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
                             .setOldLayout(ImageLayoutToNative(VKImageSrc->CurrentLayout))
                             .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                             .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                             .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                             .setImage(VKImageSrc->Handle)
                             .setSubresourceRange(SourceRange);

        VkCommandBufferData->Handle.pipelineBarrier(
            ImageLayoutToPipelineStage(VKImageSrc->CurrentLayout),
            vk::PipelineStageFlagBits::eTransfer,
            {},
            {},
            {},
            ToTransferSrcBarrier
        );
    }


    auto DestLayers = GetDefaultImageSubresourceLayers(*Destination.Resource, Destination.MipLevel, Destination.Layer, Destination.Layercount);
    auto SourceLayers = GetDefaultImageSubresourceLayers(*SrcImage, 0, 0, 1);
    vk::ImageCopy ImageCopyInfo;
    ImageCopyInfo.setSrcSubresource(SourceLayers)
                 .setDstSubresource(DestLayers)
                 .setExtent(vk::Extent3D(SrcImage->Extent.Width, SrcImage->Extent.Height, 1))
                 .setSrcOffset(vk::Offset3D(0,0,0))
                 .setDstOffset(vk::Offset3D(0,0,0));
    VkCommandBufferData->Handle.copyImage(VKImageSrc->Handle, vk::ImageLayout::eTransferSrcOptimal, VKImageDest->Handle, vk::ImageLayout::eTransferDstOptimal, 1, &ImageCopyInfo);

    TransferLayout(*Destination.Resource, imageLayout::TransferDstOptimal, DstInitialUsage);
    TransferLayout(*SrcImage, imageLayout::TransferSrcOptimal, SrcInitialUsage);
}  

void commandBuffer::CopyBufferToImage(const bufferInfo &Source, const imageInfo &Destination)
{
    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, this);
    GET_API_DATA(VKImage, vkImageData, Destination.Resource);
    GET_API_DATA(VKBuffer, vkBufferData, Source.Resource);

    if(Destination.Usage != imageUsage::TRANSFER_DESTINATION)
    {
        auto DestinationRange = GetDefaultImageSubresourceRange(*Destination.Resource);
        vk::ImageMemoryBarrier ToTransferDestBarrier;
        ToTransferDestBarrier.setSrcAccessMask(ImageUsageToAccessFlags(Destination.Usage))
                             .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                             .setOldLayout(ImageUsageToImageLayoutNative(Destination.Usage))
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

    auto DestinationLayers = GetDefaultImageSubresourceLayers(*Destination.Resource, Destination.MipLevel, Destination.Layer, Destination.Layercount);
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

void commandBuffer::TransferLayout(imageHandle Texture, imageUsage::bits OldLayout, imageUsage::bits NewLayout)
{
    image *Image = context::Get()->GetImage(Texture);
    TransferLayout(*Image, OldLayout, NewLayout);
}

void commandBuffer::TransferLayout(const image &Texture, imageLayout OldLayout, imageLayout NewLayout)
{
    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, this);
    GET_API_DATA(VKImage, vkImageData, (&Texture));

    auto Barrier = GetImageMemoryBarrier(Texture, OldLayout, NewLayout);
    VkCommandBufferData->Handle.pipelineBarrier(
        ImageLayoutToPipelineStage(OldLayout),
        ImageLayoutToPipelineStage(NewLayout),
        {},
        {},
        {},
        Barrier
    );    
    VKImage->CurrentLayout = NewLayout;    
}

void commandBuffer::TransferLayout(const image &Texture, imageUsage::bits OldLayout, imageUsage::bits NewLayout)
{
    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, this);
    GET_API_DATA(VKImage, vkImageData, (&Texture));

    auto Barrier = GetImageMemoryBarrier(Texture, OldLayout, NewLayout);
    VkCommandBufferData->Handle.pipelineBarrier(
        ImageUsageToPipelineStage(OldLayout),
        ImageUsageToPipelineStage(NewLayout),
        {},
        {},
        {},
        Barrier
    );    
    VKImage->CurrentLayout =  ImageUsageToImageLayout(NewLayout);
}





void commandBuffer::BindUniformGroup(std::shared_ptr<uniformGroup> Group, u32 Binding)
{   
    GET_API_DATA(VkCommandBufferData, vkCommandBufferData, this);
    pipeline *Pipeline = context::Get()->GetPipeline(VkCommandBufferData->BoundPipeline);
    GET_API_DATA(VkPipeline, vkPipelineData, Pipeline);
    GET_API_DATA(VkUniformData, vkUniformData, Group);
    VkCommandBufferData->Handle.bindDescriptorSets(VkPipeline->BindPoint, VkPipeline->PipelineLayout, Binding, 1, &VkUniformData->DescriptorInfos[Pipeline->Name].DescriptorSet, 0, 0);
}



}

#endif