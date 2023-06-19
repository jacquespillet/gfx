#pragma once
#include <vulkan/vulkan.hpp>
#include "../Include/CommandBuffer.h"

namespace gfx
{

struct vkCommandBufferData
{
    vk::CommandBuffer Handle;
};

commandBuffer *CreateVkCommandBuffer(vk::CommandBuffer VkCommandBuffer)
{
    commandBuffer *CommandBuffer = new commandBuffer();
    CommandBuffer->VkData = new vkCommandBufferData();
    vkCommandBufferData *VkData = (vkCommandBufferData*)CommandBuffer->VkData;
    VkData->Handle = VkCommandBuffer;

    return CommandBuffer;
}

}