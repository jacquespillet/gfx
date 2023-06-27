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
    std::vector<image*> SwapchainImages;
    std::vector<imageUsage::bits> SwapchainImageUsages;
    std::vector<vk::Framebuffer> Framebuffers;

    uint32_t CurrentIndex=0;
    
    image *AcquireSwapchainImage(size_t Index, imageUsage::bits Usage)
    {
        this->SwapchainImageUsages[Index] = Usage;
        return this->SwapchainImages[Index];
    }

};

}