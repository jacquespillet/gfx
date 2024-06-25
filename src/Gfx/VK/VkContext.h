#pragma once


#include "../Include/Types.h"
#include "../Include/CommandBuffer.h"
#include "../Include/Image.h"
#include "../Include/Buffer.h"
#include "VkMapping.h"


#include <vulkan/vulkan.hpp>
#include <unordered_map>
#include <string>

#include "VkMemoryAllocation.h"
#include "VkVmaUsage.h"

#define VK_CALL(f)\
{\
    VkResult Res = (f); \
    if(Res != VK_SUCCESS) \
    { \
        assert(0); \
    } \
} \


namespace gfx
{
class virtualFrameProvider;

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

    vk::DescriptorPool DescriptorPool;

    vk::CommandPool CommandPool;
    std::shared_ptr<commandBuffer> ImmediateCommandBuffer {};

    stageBuffer StageBuffer;

    vk::Extent2D SurfaceExtent;

    std::shared_ptr<virtualFrameProvider> VirtualFrames;

    //Multisampling
    b8 MultisamplingEnabled=false;

    u64 GetBufferDeviceAddress(bufferHandle Buffer);

    // RTX
    // vk::DescriptorPool BindlessDescriptorPool;
    
    static const u32 BindlessTexturesCount = 1024;
    static const u32 BindlessUniformBuffersCount = 1024;

    vk::DescriptorSetLayout  BindlessDescriptorSetLayout;
    vk::DescriptorSet BindlessDescriptorSet;
    
    // vk::DescriptorSetLayout  BindlessBuffersDescriptorSetLayout;
    // vk::DescriptorSet BindlessBufferDescriptorSet;
        

    vk::PhysicalDeviceRayTracingPipelinePropertiesKHR RayTracingPipelineProperties;
    vk::PhysicalDeviceAccelerationStructureFeaturesKHR AccelerationStructureFeatures;
    void *DevicePNextChain=nullptr;
    VkPhysicalDeviceRayQueryFeaturesKHR EnabledRayQueryFeatures{};
	VkPhysicalDeviceDescriptorIndexingFeaturesEXT EnabledDescriptorIndexingFeatures{};
	VkPhysicalDeviceBufferDeviceAddressFeatures EnabledBufferDeviceAddresFeatures{};
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR EnabledRayTracingPipelineFeatures{};
	VkPhysicalDeviceAccelerationStructureFeaturesKHR EnabledAccelerationStructureFeatures{};
    
    PFN_vkCreateRayTracingPipelinesKHR _vkCreateRayTracingPipelinesKHR;
    PFN_vkGetBufferDeviceAddressKHR _vkGetBufferDeviceAddressKHR;
    PFN_vkGetAccelerationStructureBuildSizesKHR _vkGetAccelerationStructureBuildSizesKHR;
    PFN_vkCreateAccelerationStructureKHR _vkCreateAccelerationStructureKHR;
    PFN_vkGetAccelerationStructureDeviceAddressKHR _vkGetAccelerationStructureDeviceAddressKHR;
    PFN_vkBuildAccelerationStructuresKHR _vkBuildAccelerationStructuresKHR;
    PFN_vkCmdBuildAccelerationStructuresKHR _vkCmdBuildAccelerationStructuresKHR;
    PFN_vkGetRayTracingShaderGroupHandlesKHR _vkGetRayTracingShaderGroupHandlesKHR;
    PFN_vkCmdTraceRaysKHR _vkCmdTraceRaysKHR;
    PFN_vkDestroyAccelerationStructureKHR _vkDestroyAccelerationStructureKHR;
};

stageBuffer CreateVkStageBuffer(sz Size);

}