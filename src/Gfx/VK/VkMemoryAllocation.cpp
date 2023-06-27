#if GFX_API==GFX_VK
#include "VkMemoryAllocation.h"
#include "../Include/GfxContext.h"
#include "VkGfxContext.h"

namespace gfx
{
VmaMemoryUsage MemoryUsageToNative(memoryUsage Usage)
{
    constexpr VmaMemoryUsage MappingTable[] = 
    {
        VMA_MEMORY_USAGE_GPU_ONLY,
        VMA_MEMORY_USAGE_CPU_ONLY,
        VMA_MEMORY_USAGE_CPU_TO_GPU,
        VMA_MEMORY_USAGE_GPU_TO_CPU,
        VMA_MEMORY_USAGE_CPU_COPY,
        VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED
    };
    return MappingTable[(size_t)Usage];
}

VmaAllocation AllocateBuffer(const vk::BufferCreateInfo &BufferCreateInfo, memoryUsage Usage, vk::Buffer *Buffer)
{
    vkData *VkData =(vkData *)context::Get()->ApiContextData;

    VmaAllocation Allocation = {};
    VmaAllocationCreateInfo AllocationCreateInfo = {};
    AllocationCreateInfo.usage = MemoryUsageToNative(Usage);
    (void)vmaCreateBuffer(VkData->Allocator, (VkBufferCreateInfo*)&BufferCreateInfo, &AllocationCreateInfo, (VkBuffer*)Buffer, &Allocation, nullptr);
    return Allocation;
}

void DeallocateBuffer(const vk::Buffer &Buffer, VmaAllocation Allocation)
{
    vkData *VkData =(vkData *)context::Get()->ApiContextData;
    vmaDestroyBuffer(VkData->Allocator, Buffer, Allocation);
}

    
u8 *MapMemory(VmaAllocation Allocation)
{
    
    vkData *VkData =(vkData *)context::Get()->ApiContextData;
    void *Memory = nullptr;
    vmaMapMemory(VkData->Allocator, Allocation, &Memory);
    return (u8*)Memory;
    
}

void UnmapMemory(VmaAllocation Allocation)
{
    vkData *VkData =(vkData *)context::Get()->ApiContextData;
    vmaUnmapMemory(VkData->Allocator, Allocation);
}

void FlushMemory(VmaAllocation Allocation, size_t ByteSize, size_t Offset)
{
    vkData *VkData =(vkData *)context::Get()->ApiContextData;
    vmaFlushAllocation(VkData->Allocator, Allocation, Offset, ByteSize);
}

VmaAllocation AllocateImage(const vk::ImageCreateInfo &ImageCreateInfo, memoryUsage Usage, vk::Image *Image)
{
    vkData *VkData =(vkData *)context::Get()->ApiContextData;
    VmaAllocation Allocation = {};
    VmaAllocationCreateInfo AllocationInfo = {};
    AllocationInfo.usage = MemoryUsageToNative(Usage);
    (void)vmaCreateImage(VkData->Allocator, (VkImageCreateInfo*)&ImageCreateInfo, &AllocationInfo, (VkImage*)Image, &Allocation, nullptr);

    return Allocation;
}
    

}

#endif