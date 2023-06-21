#pragma once
#include "../Include/Types.h"
#include "../Include/CommandBuffer.h"
#include "../Include/Image.h"
#include "../Include/Buffer.h"
#include "VkMapping.h"

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
namespace gfx
{

struct vkData
{
    vk::Instance Instance;
    u32 ApiVersion;
    vk::SurfaceKHR Surface;
    
    vk::PhysicalDevice PhysicalDevice;
    vk::PhysicalDeviceProperties PhysicalDeviceProperties;
    
    u32 QueueFamilyIndex;

    vk::PresentModeKHR SurfacePresentMode;
    u32 PresentImageCount;
    vk::SurfaceFormatKHR SurfaceFormat;

    vk::Device Device;
    vk::Queue DeviceQueue;

    vk::DispatchLoaderDynamic DynamicLoader;

    vk::DebugUtilsMessengerEXT DebugUtilsMessenger;

    VmaAllocator Allocator;

    vk::Semaphore ImageAvailableSemaphore;
    vk::Semaphore RenderingFinishedSemaphore;
    vk::Fence ImmediateFence;

    vk::CommandPool CommandPool;
    commandBuffer *ImmediateCommandBuffer {};

    stageBuffer StageBuffer;

    vk::Extent2D SurfaceExtent;
};

}