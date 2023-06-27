#if API == VK
#include "VkImage.h"
#include "../Include/GfxContext.h"
#include "../Include/Memory.h"
#include "VkGfxContext.h"
#include "VkCommon.h"
#include "VkMemoryAllocation.h"
namespace gfx
{


image *CreateImage(vk::Image VkImage, u32 Width, u32 Height, format Format)
{
    image *Image = (image*)AllocateMemory(sizeof(image));
    *Image = image();
    Image->ApiData = (vkImageData*)AllocateMemory(sizeof(vkImageData));

    Image->Extent.Width = Width;
    Image->Extent.Height = Height;
    Image->Format = Format;

    vkImageData *ImageData = (vkImageData*)Image->ApiData;
    ImageData->Allocation = {};
    ImageData->InitViews(*Image, VkImage, Format);

    return Image;
}





image *CreateEmptyImage(u32 Width, u32 Height, format Format, imageUsage::value ImageUsage, memoryUsage MemoryUsage)
{
    image *Image = (image*)AllocateMemory(sizeof(image));
    *Image = image();
    Image->ApiData = (vkImageData*)AllocateMemory(sizeof(vkImageData));
    vkImageData *VkImageData = (vkImageData*)Image->ApiData;
    
    Image->MipLevelCount = 1;
    Image->LayerCount = 1;
    Image->Format = Format;
    Image->Extent.Width = Width;
    Image->Extent.Height = Height;
    

    vk::ImageCreateInfo ImageCreateInfo;
    ImageCreateInfo.setImageType(vk::ImageType::e2D)
                    .setFormat(FormatToNative(Format))
                    .setExtent(vk::Extent3D(Width, Height, 1))
                    .setSamples(vk::SampleCountFlagBits::e1)
                    .setMipLevels(Image->MipLevelCount)
                    .setArrayLayers(Image->LayerCount)
                    .setTiling(vk::ImageTiling::eOptimal)
                    .setUsage((vk::ImageUsageFlags)ImageUsage)
                    .setSharingMode(vk::SharingMode::eExclusive)
                    .setInitialLayout(vk::ImageLayout::eUndefined);

    // if(Options & imageOptions::CUBEMAP)
    // {
    //     ImageCreateInfo.setFlags(vk::ImageCreateFlagBits::eCubeCompatible);
    // }                       

    VkImageData->Allocation = gfx::AllocateImage(ImageCreateInfo, MemoryUsage, &VkImageData->Handle);
    VkImageData->InitViews(*Image, VkImageData->Handle,  Format);

    return Image;    
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
    }

    //If stencil image,
    auto StencilSubresourceRange = GetDefaultImageSubresourceRange(Image);
    StencilSubresourceRange.setAspectMask(StencilSubresourceRange.aspectMask & vk::ImageAspectFlagBits::eStencil);
    if(StencilSubresourceRange.aspectMask != vk::ImageAspectFlags())
    {
        ImageViewCreateInfo.setSubresourceRange(StencilSubresourceRange);
        this->DefaultImageViews.StencilOnlyView = VkData->Device.createImageView(ImageViewCreateInfo);
    }    
}

}

#endif