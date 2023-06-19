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
};

struct vkImageData
{
    imageViews DefaultImageViews;
    std::vector<imageViews> CubemapImageViews;
    vk::Image Handle;
    VmaAllocation Allocation = {};
    vk::Sampler Sampler;

    void InitViews(const vk::Image &Image, format Format);
};

image *CreateImage(vk::Image VkImage, u32 Width, u32 Height, format Format)
{
    image *Image = new image();
    Image->VkData = new vkImageData();
    Image->Extent.Width = Width;
    Image->Extent.Height = Height;
    Image->Format = Format;

    vkImageData *ImageData = (vkImageData*)Image->VkData;
    ImageData->Allocation = {};
    ImageData->InitViews(VkImage, Format);

    return Image;
}

}