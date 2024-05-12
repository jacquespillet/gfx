#pragma once
#include <vulkan/vulkan.hpp>
#include "VkCommon.h"
#include "VkContext.h"

namespace gfx
{
struct buffer;

struct vkBufferData
{
    vk::Buffer Handle;
    VmaAllocation Allocation;
};



vk::Buffer GetBufferHandle(buffer *Buffer);

}