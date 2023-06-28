#pragma once
#include "../Include/Types.h"
#include "../Include/CommandBuffer.h"
#include "../Include/Image.h"
#include "../Include/Buffer.h"
#include "vkVirtualFrames.h"
#include "VkMapping.h"

#include <vulkan/vulkan.hpp>
#include <vk_mem_alloc.h>
#include <unordered_map>
#include <string>

namespace gfx
{

struct vkConstants
{
    static const u8 MaxDescriptorsPerSet = 16;
};

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
    std::shared_ptr<commandBuffer> ImmediateCommandBuffer {};

    stageBuffer StageBuffer;

    vk::Extent2D SurfaceExtent;

    virtualFrameProvider VirtualFrames;

    std::unordered_map<std::string, renderPassHandle> RenderPassCache;

    renderPass *GetRenderPass(const renderPassOutput &Output, std::string Name);
    
};

stageBuffer CreateVkStageBuffer(sz Size);

}