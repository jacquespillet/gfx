#pragma once
#include <vulkan/vulkan.hpp>
#include <vector>

#include "Mapping.h"

namespace gfx
{
struct image;
struct vkSwapchainData
{
    vk::SwapchainKHR Handle;
    std::vector<image*> SwapchainImages;
    std::vector<imageUsage::bits> SwapchainImageUsages;
};

}