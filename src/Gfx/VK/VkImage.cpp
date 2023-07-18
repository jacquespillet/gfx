#if GFX_API == GFX_VK
#include "VkImage.h"
#include "../Include/GfxContext.h"
#include "../Include/Memory.h"
#include "VkGfxContext.h"
#include "VkCommon.h"
#include "VkMemoryAllocation.h"

#include <algorithm>
namespace gfx
{


void GenerateMipmaps(vk::Image Image, u32 Width, u32 Height, u32 MipLevels, vk::CommandBuffer &CommandBuffer)
{
    // VkImageMemoryBarrier Barrier {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    vk::ImageMemoryBarrier Barrier;
    Barrier.image = Image;
    Barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    Barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    Barrier.subresourceRange.baseArrayLayer=0;
    Barrier.subresourceRange.layerCount=1;
    Barrier.subresourceRange.levelCount=1;

    int32_t MipWidth = Width;
    int32_t MipHeight = Height;
    for(u32 i=1; i<MipLevels; i++)
    {
        Barrier.subresourceRange.baseMipLevel=i-1;
        Barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        Barrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
        Barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        Barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        CommandBuffer.pipelineBarrier(
            vk::PipelineStageFlagBits::eTransfer, 
            vk::PipelineStageFlagBits::eTransfer, (vk::DependencyFlagBits)0,
            0, nullptr, 
            0, nullptr, 
            1, &Barrier);
        
        vk::ImageBlit Blit {};
        Blit.srcOffsets[0] = vk::Offset3D(0,0,0);
        Blit.srcOffsets[1] = vk::Offset3D(MipWidth, MipHeight, 1);
        Blit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        Blit.srcSubresource.mipLevel = i-1;
        Blit.srcSubresource.baseArrayLayer = 0;
        Blit.srcSubresource.layerCount=1;
        Blit.dstOffsets[0] = vk::Offset3D(0,0,0);
        Blit.dstOffsets[1] = vk::Offset3D(MipWidth > 1 ? MipWidth / 2 : 1, MipHeight > 1 ? MipHeight/2 : 1, 1);
        Blit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        Blit.dstSubresource.mipLevel = i;
        Blit.dstSubresource.baseArrayLayer = 0;
        Blit.dstSubresource.layerCount=1;

        CommandBuffer.blitImage(Image, vk::ImageLayout::eTransferSrcOptimal,
                                Image, vk::ImageLayout::eTransferDstOptimal,
                                1, &Blit, vk::Filter::eLinear);
        
        Barrier.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
        Barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        Barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        Barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        CommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, (vk::DependencyFlagBits)0,
                                     0, nullptr,
                                     0, nullptr,
                                     1, &Barrier);
        if (MipWidth > 1) MipWidth /= 2;
        if (MipHeight > 1) MipHeight /= 2;                     
    }

    Barrier.subresourceRange.baseMipLevel = MipLevels - 1;
    Barrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
    Barrier.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
    Barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
    Barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    
    CommandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, (vk::DependencyFlagBits)0,
                                    0, nullptr,
                                    0, nullptr,
                                    1, &Barrier);
}

void vkImageData::Init(const image &Image, imageUsage::value ImageUsage, memoryUsage MemoryUsage)
{
    vk::ImageCreateInfo ImageCreateInfo;
    ImageCreateInfo.setImageType(vk::ImageType::e2D)
                    .setFormat(FormatToNative(Image.Format))
                    .setExtent(vk::Extent3D(Image.Extent.Width, Image.Extent.Height, 1))
                    .setSamples(vk::SampleCountFlagBits::e1)
                    .setMipLevels(Image.MipLevelCount)
                    .setArrayLayers(Image.LayerCount)
                    .setTiling(vk::ImageTiling::eOptimal)
                    .setUsage((vk::ImageUsageFlags)ImageUsage)
                    .setSharingMode(vk::SharingMode::eExclusive)
                    .setInitialLayout(vk::ImageLayout::eUndefined);

    // if(Options & imageOptions::CUBEMAP)
    // {
    //     ImageCreateInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
    // }                       

    Allocation = gfx::AllocateImage(ImageCreateInfo, MemoryUsage, &Handle);
    InitViews(Image, Handle, Image.Format);
}

void image::Init(const imageData &ImageData, const imageCreateInfo &CreateInfo)
{
    Extent.Width = ImageData.Width;
    Extent.Height = ImageData.Height;
    Format = ImageData.Format;


    MipLevelCount = CreateInfo.GenerateMipmaps ? static_cast<u32>(std::floor(std::log2((std::max)(this->Extent.Width, this->Extent.Height)))) + 1 : 1;

    context *VulkanContext = context::Get();
    ApiData = std::make_shared<vkImageData>();
    std::shared_ptr<vkImageData> VKImage = std::static_pointer_cast<vkImageData>(ApiData);


    imageUsage::value ImageUsage = imageUsage::TRANSFER_DESTINATION | imageUsage::SHADER_READ; 
    if(CreateInfo.GenerateMipmaps) ImageUsage |= imageUsage::TRANSFER_SOURCE;
    VKImage->Init(
        *this,
        ImageUsage,
        memoryUsage::GpuOnly
    );

    // CommandBuffer.Begin();
    GET_CONTEXT(VkData, context::Get());
    auto TextureAllocation = VkData->StageBuffer.Submit(ImageData.Data, (u32)ImageData.DataSize);
    
    VkData->ImmediateCommandBuffer->Begin();

    
    VkData->ImmediateCommandBuffer->CopyBufferToImage(
        bufferInfo {VkData->StageBuffer.GetBuffer(), TextureAllocation.Offset },
        imageInfo {this, imageUsage::UNKNOWN, 0, 0}
    );

    if(!CreateInfo.GenerateMipmaps)
        VkData->ImmediateCommandBuffer->TransferLayout(*this, imageUsage::TRANSFER_DESTINATION, imageUsage::SHADER_READ);
    else
    {
        GenerateMipmaps(VKImage->Handle, this->Extent.Width, this->Extent.Height, MipLevelCount, std::static_pointer_cast<vkCommandBufferData>(VkData->ImmediateCommandBuffer->ApiData)->Handle);
    }

    VkData->StageBuffer.Flush();
    VkData->ImmediateCommandBuffer->End();
    context::Get()->SubmitCommandBufferImmediate(VkData->ImmediateCommandBuffer.get());
    VkData->StageBuffer.Reset();

    VKImage->InitSampler(CreateInfo, MipLevelCount);
}

image::image(vk::Image VkImage, u32 Width, u32 Height, format Format)
{
    this->ApiData = std::make_shared<vkImageData>();

    this->Extent.Width = Width;
    this->Extent.Height = Height;
    this->Format = Format;

    std::shared_ptr<vkImageData> ImageData = std::static_pointer_cast<vkImageData>(ApiData);
    ImageData->Allocation = {};
    ImageData->InitViews(*this, VkImage, Format);
}





void image::Init(u32 Width, u32 Height, format Format, imageUsage::value ImageUsage, memoryUsage MemoryUsage, u32 SampleCount)
{
    ApiData = std::make_shared<vkImageData>();
    std::shared_ptr<vkImageData> VkImageData = std::static_pointer_cast<vkImageData>(ApiData);
    
    this->MipLevelCount = 1;
    this->LayerCount = 1;
    this->Format = Format;
    this->Extent.Width = Width;
    this->Extent.Height = Height;
    

    vk::ImageCreateInfo ImageCreateInfo;
    ImageCreateInfo.setImageType(vk::ImageType::e2D)
                    .setFormat(FormatToNative(Format))
                    .setExtent(vk::Extent3D(Width, Height, 1))
                    .setSamples(SampleCountToNative(SampleCount))
                    .setMipLevels(MipLevelCount)
                    .setArrayLayers(LayerCount)
                    .setTiling(vk::ImageTiling::eOptimal)
                    .setUsage((vk::ImageUsageFlags)ImageUsage)
                    .setSharingMode(vk::SharingMode::eExclusive)
                    .setInitialLayout(vk::ImageLayout::eUndefined);
        
    // if(Options & imageOptions::CUBEMAP)
    // {
    //     ImageCreateInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
    // }                       

    VkImageData->Allocation = gfx::AllocateImage(ImageCreateInfo, MemoryUsage, &VkImageData->Handle);
    VkImageData->InitViews(*this, VkImageData->Handle,  Format);
}

vk::ImageSubresourceRange GetDefaultImageSubresourceRange(const image &Image)
{
    return vk::ImageSubresourceRange(
        ImageFormatToImageAspect(Image.Format),
        0,
        Image.MipLevelCount,
        0,
        Image.LayerCount
    );    
}

vk::ImageSubresourceLayers GetDefaultImageSubresourceLayers(const image &Image, u32 MipLevel, u32 Layer)
{
    return vk::ImageSubresourceLayers(
        ImageFormatToImageAspect(Image.Format),
        MipLevel,
        Layer,
        1
    );
}


vk::ImageMemoryBarrier GetImageMemoryBarrier(const image &Texture, imageUsage::bits OldLayout, imageUsage::bits NewLayout)
{
    std::shared_ptr<vkImageData> VkImageData = std::static_pointer_cast<vkImageData>(Texture.ApiData);

    auto SubResourceRange = GetDefaultImageSubresourceRange(Texture);
    vk::ImageMemoryBarrier Barrier;
    Barrier.setSrcAccessMask(ImageUsageToAccessFlags(OldLayout))
           .setDstAccessMask(ImageUsageToAccessFlags(NewLayout))
           .setOldLayout(ImageUsageToImageLayout(OldLayout))
           .setNewLayout(ImageUsageToImageLayout(NewLayout))
           .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
           .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
           .setImage(VkImageData->Handle)
           .setSubresourceRange(SubResourceRange);

    return Barrier;
}


void vkImageData::InitViews(const image &Image, const vk::Image &VkImage, format Format)
{
    auto Vulkan = context::Get();
    GET_CONTEXT(VkData, Vulkan);

    this->Handle = VkImage;
    
    auto SubresourceRange = GetDefaultImageSubresourceRange(Image);

    //Create image view
    vk::ImageViewCreateInfo ImageViewCreateInfo;
    ImageViewCreateInfo.setImage(this->Handle)
                        .setViewType(vk::ImageViewType::e2D)
                        .setFormat(FormatToNative(Format))
                        .setComponents(vk::ComponentMapping(vk::ComponentSwizzle::eIdentity,
                                                            vk::ComponentSwizzle::eIdentity,
                                                            vk::ComponentSwizzle::eIdentity,
                                                            vk::ComponentSwizzle::eIdentity))
                        .setSubresourceRange(SubresourceRange);

    auto NativeSubresourceRange = GetDefaultImageSubresourceRange(Image);
    ImageViewCreateInfo.setSubresourceRange(NativeSubresourceRange);
    this->DefaultImageViews.NativeView = VkData->Device.createImageView(ImageViewCreateInfo);

    //If depth image,
    auto DepthSubresourceRange = GetDefaultImageSubresourceRange(Image);
    DepthSubresourceRange.setAspectMask(DepthSubresourceRange.aspectMask & vk::ImageAspectFlagBits::eDepth);
    if(DepthSubresourceRange.aspectMask != vk::ImageAspectFlags())
    {
        ImageViewCreateInfo.setSubresourceRange(DepthSubresourceRange);
        this->DefaultImageViews.DepthOnlyView = VkData->Device.createImageView(ImageViewCreateInfo);
        this->DefaultImageViews.DepthOnlyViewSet=true;
    }

    //If stencil image,
    auto StencilSubresourceRange = GetDefaultImageSubresourceRange(Image);
    StencilSubresourceRange.setAspectMask(StencilSubresourceRange.aspectMask & vk::ImageAspectFlagBits::eStencil);
    if(StencilSubresourceRange.aspectMask != vk::ImageAspectFlags())
    {
        ImageViewCreateInfo.setSubresourceRange(StencilSubresourceRange);
        this->DefaultImageViews.StencilOnlyView = VkData->Device.createImageView(ImageViewCreateInfo);
        this->DefaultImageViews.StencilOnlyViewSet=true;
    }    
}

void vkImageData::InitSampler(const imageCreateInfo &CreateInfo, u32 MipLevelCount)
{
    vk::SamplerCreateInfo SamplerCreateInfo;
    SamplerCreateInfo.setMagFilter(SamplerFilterToNative(CreateInfo.MinFilter))
                     .setMinFilter(SamplerFilterToNative(CreateInfo.MagFilter))
                     .setMipmapMode(SamplerFilterToNativeMip(CreateInfo.MinFilter))
                     .setAddressModeU(SamplerWrapModeToNative(CreateInfo.WrapR))
                     .setAddressModeV(SamplerWrapModeToNative(CreateInfo.WrapS))
                     .setAddressModeW(SamplerWrapModeToNative(CreateInfo.WrapT))
                     .setMinLod(0)
                     .setMaxLod(MipLevelCount)
                     .setBorderColor(vk::BorderColor::eFloatOpaqueWhite);
    
    auto Vulkan = context::Get();
    GET_CONTEXT(VkData, context::Get());

    this->Sampler = VkData->Device.createSampler(SamplerCreateInfo);
}

void image::Destroy()
{
    GET_CONTEXT(VkData, context::Get());
    std::shared_ptr<vkImageData> VkImageData = std::static_pointer_cast<vkImageData>(ApiData);

    VkData->Device.destroyImageView(VkImageData->DefaultImageViews.NativeView);
    if(VkImageData->DefaultImageViews.DepthOnlyViewSet)
        VkData->Device.destroyImageView(VkImageData->DefaultImageViews.DepthOnlyView);
    if(VkImageData->DefaultImageViews.StencilOnlyViewSet)
        VkData->Device.destroyImageView(VkImageData->DefaultImageViews.StencilOnlyView);

    if((void*)VkImageData->Sampler != nullptr)
    {
        VkData->Device.destroySampler(VkImageData->Sampler);
    }
    
    vmaDestroyImage(VkData->Allocator, VkImageData->Handle, VkImageData->Allocation);

}

}

#endif