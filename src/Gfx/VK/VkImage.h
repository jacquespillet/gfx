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

    void InitViews(const image &Image, const vk::Image &VkImage, format Format);
};


vk::ImageSubresourceRange GetDefaultImageSubresourceRange(const image &Image);


}