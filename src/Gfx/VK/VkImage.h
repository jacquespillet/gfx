#pragma once
#include "../Include/Image.h"
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace gfx
{

struct imageViews
{
    vk::ImageView NativeView;
    vk::ImageView DepthOnlyView;
    vk::ImageView StencilOnlyView;
    b8 DepthOnlyViewSet=false;
    b8 StencilOnlyViewSet=false;
};

struct vkImageData
{
    imageViews DefaultImageViews;
    std::vector<imageViews> CubemapImageViews;
    vk::Image Handle;
    VmaAllocation Allocation = {};
    vk::Sampler Sampler;

    void Init(const image &Image, imageUsage::value ImageUsage, memoryUsage MemoryUsage);
    void InitViews(const image &Image, const vk::Image &VkImage, format Format);
    void InitSampler(textureCreateInfo &CreateInfo);
};


vk::ImageSubresourceRange GetDefaultImageSubresourceRange(const image &Image);
vk::ImageSubresourceLayers GetDefaultImageSubresourceLayers(const image &Image, u32 MipLevel, u32 Layer);
vk::ImageMemoryBarrier GetImageMemoryBarrier(const image &Texture, imageUsage::bits OldLayout, imageUsage::bits NewLayout);

}