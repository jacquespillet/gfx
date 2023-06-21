#pragma once
#include <vulkan/vulkan.hpp>
#include "../Include/CommandBuffer.h"

namespace gfx
{

struct vkCommandBufferData
{
    vk::CommandBuffer Handle;
};

commandBuffer *CreateVkCommandBuffer(vk::CommandBuffer VkCommandBuffer);

}