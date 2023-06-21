#pragma once

#include <vk_mem_alloc.h>
#include <vulkan/vulkan.hpp>

#include "../Include/Types.h"
#include "VkMapping.h"

namespace gfx
{
    
VmaAllocation AllocateBuffer(const vk::BufferCreateInfo &BufferCreateInfo, memoryUsage Usage, vk::Buffer *Buffer);
VmaAllocation AllocateImage(const vk::ImageCreateInfo &ImageCreateInfo, memoryUsage Usage, vk::Image *Image);
void FlushMemory(VmaAllocation Allocation, size_t ByteSize, size_t Offset);

u8 *MapMemory(VmaAllocation Allocation);
void UnmapMemory(VmaAllocation Allocation);
void DeallocateBuffer(const vk::Buffer &Buffer, VmaAllocation Allocation);
}