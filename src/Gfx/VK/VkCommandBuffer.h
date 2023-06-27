#pragma once
#include <vulkan/vulkan.hpp>
#include "../Include/CommandBuffer.h"

namespace gfx
{

struct vkCommandBufferData
{
    vk::CommandBuffer Handle;
    vk::ClearValue Clears[2];
};

commandBuffer *CreateVkCommandBuffer(vk::CommandBuffer VkCommandBuffer);

}