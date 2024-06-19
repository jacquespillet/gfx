#pragma once
#include "../Include/Image.h"
#include <vulkan/vulkan.hpp>
#include "VkContext.h"

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

    vk::DescriptorSet ImGuiDescriptorSet;

    imageLayout CurrentLayout;

    void Init(const image &Image, imageUsage::value ImageUsage, memoryUsage MemoryUsage, vk::ImageCreateFlags Flags = (vk::ImageCreateFlags)0);
    void InitViews(const image &Image, const vk::Image &VkImage, format Format, vk::ImageViewType ViewType = vk::ImageViewType::e2D);
    void InitSampler(const imageCreateInfo &CreateInfo, u32 MipLevelCount, format Format);
    void InitSamplerDefault(format Format);
};


vk::ImageSubresourceRange GetDefaultImageSubresourceRange(const image &Image);
vk::ImageSubresourceLayers GetDefaultImageSubresourceLayers(const image &Image, u32 MipLevel, u32 LayerStart, u32 LayerCount=1);
vk::ImageMemoryBarrier GetImageMemoryBarrier(const image &Texture, imageUsage::bits OldLayout, imageUsage::bits NewLayout);
vk::ImageMemoryBarrier GetImageMemoryBarrier(const image &Texture, imageLayout OldLayout, imageLayout NewLayout);

}