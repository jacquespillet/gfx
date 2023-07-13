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
    
    std::shared_ptr<image> SwapchainImages[vkConstants::MaxSwapchainImages];
    imageUsage::bits SwapchainImageUsages[vkConstants::MaxSwapchainImages];
    framebufferHandle Framebuffers[vkConstants::MaxSwapchainImages];

    u32 ImageCount=0;

    uint32_t CurrentIndex=0;
    
    std::shared_ptr<image> AcquireSwapchainImage(size_t Index, imageUsage::bits Usage)
    {
        this->SwapchainImageUsages[Index] = Usage;
        return this->SwapchainImages[Index];
    }

};

}