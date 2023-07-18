#pragma once
#include "../Include/Image.h"
#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>

namespace gfx
{

struct imageViews
{
    vk::ImageView NativeView = VK_NULL_HANDLE;
    vk::ImageView DepthOnlyView = VK_NULL_HANDLE;
    vk::ImageView StencilOnlyView = VK_NULL_HANDLE;
    b8 DepthOnlyViewSet=false;
    b8 StencilOnlyViewSet=false;
};

struct vkImageData
{
    imageViews DefaultImageViews;
    std::vector<imageViews> CubemapImageViews;
    vk::Image Handle;
    VmaAllocation Allocation = {};
    
    vk::Sampler Sampler = nullptr;

    void Init(const image &Image, imageUsage::value ImageUsage, memoryUsage MemoryUsage);
    void InitViews(const image &Image, const vk::Image &VkImage, format Format);
    void InitSampler(const imageCreateInfo &CreateInfo, u32 MipLevelCount);
};


vk::ImageSubresourceRange GetDefaultImageSubresourceRange(const image &Image);
vk::ImageSubresourceLayers GetDefaultImageSubresourceLayers(const image &Image, u32 MipLevel, u32 Layer);
vk::ImageMemoryBarrier GetImageMemoryBarrier(const image &Texture, imageUsage::bits OldLayout, imageUsage::bits NewLayout);

}