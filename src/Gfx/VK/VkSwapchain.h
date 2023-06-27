#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>

#include "VkMapping.h"

namespace gfx
{
struct image;
struct vkSwapchainData
{
    
    vk::SwapchainKHR Handle;
    image** SwapchainImages;
    imageUsage::bits *SwapchainImageUsages;
    framebufferHandle *Framebuffers;

    uint32_t CurrentIndex=0;
    
    image *AcquireSwapchainImage(size_t Index, imageUsage::bits Usage)
    {
        this->SwapchainImageUsages[Index] = Usage;
        return this->SwapchainImages[Index];
    }

};

}