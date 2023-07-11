#pragma once
#include <vulkan/vulkan.hpp>
#include "../Include/CommandBuffer.h"

namespace gfx
{

struct vkCommandBufferData
{
    vk::CommandBuffer Handle;

    pipelineHandle BoundPipeline;
};

std::shared_ptr<commandBuffer> CreateVkCommandBuffer(vk::CommandBuffer VkCommandBuffer);

}