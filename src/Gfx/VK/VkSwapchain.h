#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>

#include "VkMapping.h"

namespace gfx
{
struct image;
struct vkSwapchainData
{
    static const s32 maxSwapchainImages = 16;

    vk::SwapchainKHR Handle;
    std::shared_ptr<image> SwapchainImages[maxSwapchainImages];
    imageUsage::bits SwapchainImageUsages[maxSwapchainImages];
    framebufferHandle Framebuffers[maxSwapchainImages];

    u32 ImageCount=0;

    uint32_t CurrentIndex=0;
    
    std::shared_ptr<image> AcquireSwapchainImage(size_t Index, imageUsage::bits Usage)
    {
        this->SwapchainImageUsages[Index] = Usage;
        return this->SwapchainImages[Index];
    }

};

}