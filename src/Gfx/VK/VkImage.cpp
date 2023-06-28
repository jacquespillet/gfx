#if GFX_API == GFX_VK
#include "VkImage.h"
#include "../Include/GfxContext.h"
#include "../Include/Memory.h"
#include "VkGfxContext.h"
#include "VkCommon.h"
#include "VkMemoryAllocation.h"
namespace gfx
{


image::image(vk::Image VkImage, u32 Width, u32 Height, format Format)
{
    ApiData = std::make_shared<vkImageData>();

    Extent.Width = Width;
    Extent.Height = Height;
    Format = Format;

    std::shared_ptr<vkImageData> ImageData = std::static_pointer_cast<vkImageData>(ApiData);
    ImageData->Allocation = {};
    ImageData->InitViews(*this, VkImage, Format);
}





image::image(u32 Width, u32 Height, format Format, imageUsage::value ImageUsage, memoryUsage MemoryUsage)
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
                    .setSamples(vk::SampleCountFlagBits::e1)
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

void image::Destroy()
{
    GET_CONTEXT(VkData, context::Get());
    std::shared_ptr<vkImageData> VkImageData = std::static_pointer_cast<vkImageData>(ApiData);

    VkData->Device.destroyImageView(VkImageData->DefaultImageViews.NativeView);
    if(VkImageData->DefaultImageViews.DepthOnlyViewSet)
        VkData->Device.destroyImageView(VkImageData->DefaultImageViews.DepthOnlyView);
    if(VkImageData->DefaultImageViews.StencilOnlyViewSet)
        VkData->Device.destroyImageView(VkImageData->DefaultImageViews.StencilOnlyView);
    
    vmaDestroyImage(VkData->Allocator, VkImageData->Handle, VkImageData->Allocation);

}

}

#endif