#if API == VK

#include <vulkan/vulkan.hpp>
#include <optional>
#include <iostream>
#include <algorithm>
#include <glslang/Public/ShaderLang.h>

#include "../../App/App.h"
#include "../Include/GfxContext.h"
#include "../Include/Image.h"
#include "../Include/CommandBuffer.h"
#include "../Include/Swapchain.h"
#include "../Include/Buffer.h"
#include "VkMapping.h"
#include "VkImage.h"
#include "VkCommandBuffer.h"
#include "VkGfxContext.h"
#include "VkSwapchain.h"
#include "VkBuffer.h"
#include "VkMemoryAllocation.h"
#include "VkUtil.h"
#include "VkCommon.h"


namespace gfx
{

context* context::Singleton= nullptr;

context *context::Get()
{
    return Singleton;
}

swapchain *context::CreateSwapchain(u32 Width, u32 Height)
{
    GET_CONTEXT(VkData, this);

    swapchain *Swapchain = new swapchain();

    Swapchain->ApiData = new vkSwapchainData();
    vkSwapchainData *VkSwapchainData = (vkSwapchainData*)Swapchain->ApiData;

    VkData->Device.waitIdle();

    //Get width and height
    auto SurfaceCapabilities = VkData->PhysicalDevice.getSurfaceCapabilitiesKHR(VkData->Surface);
    VkData->SurfaceExtent = vk::Extent2D(
        std::clamp(Width, SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width),
        std::clamp(Height, SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height)
    );

    //Disable rendering if surface size is 0
    if(VkData->SurfaceExtent == vk::Extent2D(0,0))
    {
        VkData->SurfaceExtent = vk::Extent2D(1,1);
        return nullptr;
    }

    //Create a swapchain
    vk::SwapchainCreateInfoKHR SwapchainCreateInfo;
    SwapchainCreateInfo.setSurface(VkData->Surface)
                       .setMinImageCount(VkData->PresentImageCount)
                       .setImageFormat(VkData->SurfaceFormat.format)
                       .setImageColorSpace(VkData->SurfaceFormat.colorSpace)
                       .setImageExtent(VkData->SurfaceExtent)
                       .setImageArrayLayers(1)
                       .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst)
                       .setImageSharingMode(vk::SharingMode::eExclusive)
                       .setPreTransform(SurfaceCapabilities.currentTransform)
                       .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                       .setPresentMode(VkData->SurfacePresentMode)
                       .setClipped(true);
    VkSwapchainData->Handle = VkData->Device.createSwapchainKHR(SwapchainCreateInfo);
    
    //Destroy old one
    if(SwapchainCreateInfo.oldSwapchain)
    {
        VkData->Device.destroySwapchainKHR(SwapchainCreateInfo.oldSwapchain);
    }
    auto SwapchainImages = VkData->Device.getSwapchainImagesKHR(VkSwapchainData->Handle);

    //reinitialize internal buffers
    VkData->PresentImageCount = (u32)SwapchainImages.size();
    VkSwapchainData->SwapchainImages.clear();
    VkSwapchainData->SwapchainImages.reserve(VkData->PresentImageCount);
    VkSwapchainData->SwapchainImageUsages.assign(VkData->PresentImageCount, imageUsage::UNKNOWN);

    //Initialize texture representations of swapchain images
    for(u32 i=0; i<VkData->PresentImageCount; i++)
    {
        VkSwapchainData->SwapchainImages.push_back(
            gfx::CreateImage(SwapchainImages[i], VkData->SurfaceExtent.width, VkData->SurfaceExtent.height, FormatFromNative(VkData->SurfaceFormat.format))
        );
    }

    return Swapchain;
}


swapchain *context::RecreateSwapchain(u32 Width, u32 Height, swapchain *OldSwapchain)
{
    GET_CONTEXT(VkData, this);

    assert(OldSwapchain);
    assert(OldSwapchain->ApiData);

    vkSwapchainData *VkSwapchainData = (vkSwapchainData*)OldSwapchain->ApiData;

    VkData->Device.waitIdle();

    //Get width and height
    auto SurfaceCapabilities = VkData->PhysicalDevice.getSurfaceCapabilitiesKHR(VkData->Surface);
    VkData->SurfaceExtent = vk::Extent2D(
        std::clamp(Width, SurfaceCapabilities.minImageExtent.width, SurfaceCapabilities.maxImageExtent.width),
        std::clamp(Height, SurfaceCapabilities.minImageExtent.height, SurfaceCapabilities.maxImageExtent.height)
    );

    //Disable rendering if surface size is 0
    if(VkData->SurfaceExtent == vk::Extent2D(0,0))
    {
        VkData->SurfaceExtent = vk::Extent2D(1,1);
        return nullptr;
    }

    //Create a swapchain
    vk::SwapchainCreateInfoKHR SwapchainCreateInfo;
    SwapchainCreateInfo.setSurface(VkData->Surface)
                       .setMinImageCount(VkData->PresentImageCount)
                       .setImageFormat(VkData->SurfaceFormat.format)
                       .setImageColorSpace(VkData->SurfaceFormat.colorSpace)
                       .setImageExtent(VkData->SurfaceExtent)
                       .setImageArrayLayers(1)
                       .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst)
                       .setImageSharingMode(vk::SharingMode::eExclusive)
                       .setPreTransform(SurfaceCapabilities.currentTransform)
                       .setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
                       .setPresentMode(VkData->SurfacePresentMode)
                       .setClipped(true)
                       .setOldSwapchain(VkSwapchainData->Handle);
    VkSwapchainData->Handle = VkData->Device.createSwapchainKHR(SwapchainCreateInfo);
    
    //Destroy old one
    if(SwapchainCreateInfo.oldSwapchain)
    {
        VkData->Device.destroySwapchainKHR(SwapchainCreateInfo.oldSwapchain);
    }
    auto SwapchainImages = VkData->Device.getSwapchainImagesKHR(VkSwapchainData->Handle);

    //reinitialize internal buffers
    VkData->PresentImageCount = (u32)SwapchainImages.size();
    VkSwapchainData->SwapchainImages.clear();
    VkSwapchainData->SwapchainImages.reserve(VkData->PresentImageCount);
    VkSwapchainData->SwapchainImageUsages.assign(VkData->PresentImageCount, imageUsage::UNKNOWN);

    //Initialize texture representations of swapchain images
    for(u32 i=0; i<VkData->PresentImageCount; i++)
    {
        VkSwapchainData->SwapchainImages.push_back(
            gfx::CreateImage(SwapchainImages[i], VkData->SurfaceExtent.width, VkData->SurfaceExtent.height, FormatFromNative(VkData->SurfaceFormat.format))
        );
    }
}


context* context::Initialize(context::initializeInfo &InitializeInfo, app::window &Window)
{
    if(Singleton==nullptr){
        Singleton = new context();
    }

    Singleton->ResourceManager.Init();

    Singleton->ApiContextData = new vkData();
    GET_CONTEXT(VkData, Singleton);
    CreateInstance(InitializeInfo, VkData);
    
    
    VkSurfaceKHR Surface;
    vk::Result Result = (vk::Result)glfwCreateWindowSurface(VkData->Instance, Window.GetHandle(), nullptr, &Surface);

    //Get the surface
    VkData->Surface = *reinterpret_cast<const VkSurfaceKHR*>(&Surface);
    if(!VkData->Surface)
    {
        InitializeInfo.ErrorCallback("Failed to initialize surface");
        return nullptr;
    }

    //Pick physical device
    InitializeInfo.InfoCallback("Physical devices :");
    auto PhysicalDevices = VkData->Instance.enumeratePhysicalDevices();
    for(const auto &PhysicalDevice : PhysicalDevices)
    {
        auto Properties = PhysicalDevice.getProperties();
        InitializeInfo.InfoCallback(std::string(Properties.deviceName.data()) + " : ");

        if(Properties.apiVersion < VkData->ApiVersion)
        {
            InitializeInfo.InfoCallback(std::string(Properties.deviceName.data()) + " : Physical device API version inferior to the required");
            InitializeInfo.InfoCallback(std::to_string(VK_VERSION_MAJOR(Properties.apiVersion)) + "." + std::to_string(VK_VERSION_MINOR(Properties.apiVersion)));
            InitializeInfo.InfoCallback(std::to_string(VK_VERSION_MAJOR(VkData->ApiVersion)) + "." + std::to_string(VK_VERSION_MINOR(VkData->ApiVersion)));
            continue;
        }

        auto QueueFamilyIndex = DetermineQueueFamilyIndex(VkData->Instance, PhysicalDevice, VkData->Surface);
        if(!QueueFamilyIndex.has_value())
        {
            InitializeInfo.InfoCallback(std::string(Properties.deviceName.data()) + " : Queue family not valid");
            continue;
        }

        VkData->PhysicalDevice = PhysicalDevice;
        VkData->PhysicalDeviceProperties = Properties;
        VkData->QueueFamilyIndex = QueueFamilyIndex.value();
        if(Properties.deviceType == DeviceTypeMapping[(u64)InitializeInfo.PreferredDeviceType]) break;
    }
    if(!VkData->PhysicalDevice)
    {
        InitializeInfo.ErrorCallback("Could not find a physical device");
        return nullptr;
    }
    else
    {
        InitializeInfo.InfoCallback((std::string("Selected physical device : ") + VkData->PhysicalDeviceProperties.deviceName.data()).c_str());
    }

    
    //Get present mode
    auto PresentModes = VkData->PhysicalDevice.getSurfacePresentModesKHR(VkData->Surface);
    VkData->SurfacePresentMode = vk::PresentModeKHR::eFifo;
    // if(std::find(PresentModes.begin(), PresentModes.end(), vk::PresentModeKHR::eMailbox) != PresentModes.end())
    // {
    //     VkData->SurfacePresentMode = vk::PresentModeKHR::eMailbox;
    // }
    InitializeInfo.InfoCallback("Selected Surface present mode " + vk::to_string(VkData->SurfacePresentMode));
    
    //Get present image count
    auto SurfaceCapabilities = VkData->PhysicalDevice.getSurfaceCapabilitiesKHR(VkData->Surface);
    VkData->PresentImageCount = std::max(SurfaceCapabilities.maxImageCount, 1u);
    InitializeInfo.InfoCallback("Present image count " + std::to_string(VkData->PresentImageCount));

    //Get surface format
    auto SurfaceFormats = VkData->PhysicalDevice.getSurfaceFormatsKHR(VkData->Surface);
    VkData->SurfaceFormat = SurfaceFormats.front();
    for(const auto &SurfaceFormat : SurfaceFormats)
    {
        if(SurfaceFormat.format == vk::Format::eR8G8B8A8Unorm || SurfaceFormat.format == vk::Format::eB8G8R8A8Unorm)
        {
            VkData->SurfaceFormat = SurfaceFormat;
        }
    }
    InitializeInfo.InfoCallback("Selected Surface format " + vk::to_string(VkData->SurfaceFormat.format));


    //////////////////////////
    //Create logical device///
    //////////////////////////

    //Queue 
    vk::DeviceQueueCreateInfo DeviceQueueCreateInfo;
    std::array QueuePriorities = {1.0f};
    DeviceQueueCreateInfo.setQueueFamilyIndex(VkData->QueueFamilyIndex);
    DeviceQueueCreateInfo.setQueuePriorities(QueuePriorities);

    //Extensions
    auto DeviceExtensions = InitializeInfo.DeviceExtensions;
    DeviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    /*DeviceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);    
    DeviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);*/

    //Extensions
    vk::PhysicalDeviceDescriptorIndexingFeatures DescriptorIndexingFeatures;
    DescriptorIndexingFeatures.descriptorBindingPartiallyBound = true;
    DescriptorIndexingFeatures.shaderInputAttachmentArrayDynamicIndexing = true;
    DescriptorIndexingFeatures.shaderUniformTexelBufferArrayDynamicIndexing = true;
    DescriptorIndexingFeatures.shaderStorageTexelBufferArrayDynamicIndexing = true;
    DescriptorIndexingFeatures.shaderUniformBufferArrayNonUniformIndexing = true;
    DescriptorIndexingFeatures.shaderSampledImageArrayNonUniformIndexing = true;
    DescriptorIndexingFeatures.shaderStorageBufferArrayNonUniformIndexing = true;
    DescriptorIndexingFeatures.shaderStorageImageArrayNonUniformIndexing = true;
    DescriptorIndexingFeatures.shaderInputAttachmentArrayDynamicIndexing = true;
    DescriptorIndexingFeatures.shaderUniformTexelBufferArrayNonUniformIndexing = true;
    DescriptorIndexingFeatures.descriptorBindingUniformBufferUpdateAfterBind = true;
    DescriptorIndexingFeatures.descriptorBindingSampledImageUpdateAfterBind = true;
    DescriptorIndexingFeatures.descriptorBindingStorageImageUpdateAfterBind = true;
    DescriptorIndexingFeatures.descriptorBindingStorageBufferUpdateAfterBind = true;
    DescriptorIndexingFeatures.descriptorBindingUniformTexelBufferUpdateAfterBind = true;
    DescriptorIndexingFeatures.descriptorBindingStorageTexelBufferUpdateAfterBind = true;


    vk::DeviceCreateInfo DeviceCreateInfo;
    DeviceCreateInfo.setQueueCreateInfos(DeviceQueueCreateInfo)
                    .setPEnabledExtensionNames(DeviceExtensions)
                    //.setPNext(&DescriptorIndexingFeatures)
                    ;

    VkData->Device = VkData->PhysicalDevice.createDevice(DeviceCreateInfo);
    VkData->DeviceQueue = VkData->Device.getQueue(VkData->QueueFamilyIndex, 0);

    InitializeInfo.InfoCallback("Created device and queues");

    //Initialize dynamic dispatch loader
    VkData->DynamicLoader.init(VkData->Instance, VkData->Device);

    //Setup debug utils messenger
    vk::DebugUtilsMessengerCreateInfoEXT DebugUtilsMessengerCreateInfo;
    DebugUtilsMessengerCreateInfo.setMessageSeverity(vk::DebugUtilsMessageSeverityFlagBitsEXT::eError | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
                                 .setMessageType(vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance)
                                 .setPfnUserCallback(ValidationLayerCallback);
    VkData->DebugUtilsMessenger = VkData->Instance.createDebugUtilsMessengerEXT(DebugUtilsMessengerCreateInfo, nullptr, VkData->DynamicLoader);

    //Initialize VMA
    VmaAllocatorCreateInfo AllocatorInfo = {};
    AllocatorInfo.vulkanApiVersion = VkData->ApiVersion;
    AllocatorInfo.physicalDevice = VkData->PhysicalDevice;
    AllocatorInfo.device = VkData->Device;
    AllocatorInfo.instance = VkData->Instance;
    vmaCreateAllocator(&AllocatorInfo, &VkData->Allocator);
    InitializeInfo.InfoCallback("Created VMA");


    //Initialize glslang
    glslang::InitializeProcess();
    InitializeInfo.InfoCallback("Initialized GLSL compiler");

    
    //Initialize sync objects
    VkData->ImageAvailableSemaphore = VkData->Device.createSemaphore(vk::SemaphoreCreateInfo());
    VkData->RenderingFinishedSemaphore = VkData->Device.createSemaphore(vk::SemaphoreCreateInfo());
    VkData->ImmediateFence = VkData->Device.createFence(vk::FenceCreateInfo());

    //Initialize command pool
    vk::CommandPoolCreateInfo CommandPoolCreateInfo;
    CommandPoolCreateInfo.setQueueFamilyIndex(VkData->QueueFamilyIndex)
                         .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer | vk::CommandPoolCreateFlagBits::eTransient);
    VkData->CommandPool = VkData->Device.createCommandPool(CommandPoolCreateInfo);


    //Initialized command buffer
    vk::CommandBufferAllocateInfo CommandBufferAllocateInfo;
    CommandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::ePrimary)
                             .setCommandPool(VkData->CommandPool)
                             .setCommandBufferCount(1);
    
    VkData->ImmediateCommandBuffer = CreateVkCommandBuffer(VkData->Device.allocateCommandBuffers(CommandBufferAllocateInfo).front());
    InitializeInfo.InfoCallback("Created Command Buffer");

    VkData->StageBuffer = Singleton->CreateStageBuffer(InitializeInfo.MaxStageBufferSize);

    // //Initialize descriptor cache
    // VkData->DescriptorCache.Init();
    
    //Initialize virtual frames
    // VkData->VirtualFrames.Init(InitializeInfo.VirtualFrameCount, InitializeInfo.MaxStageBufferSize);

    InitializeInfo.InfoCallback("Initialization Finished");    

    return Singleton;
}

commandBuffer *context::CreateCommandBuffer()
{
    GET_CONTEXT(VkData, this);
    vk::CommandBufferAllocateInfo CommandBufferAllocateInfo;
    CommandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::ePrimary)
                             .setCommandPool(VkData->CommandPool)
                             .setCommandBufferCount(1);
    
    commandBuffer *CommandBuffer = CreateVkCommandBuffer(VkData->Device.allocateCommandBuffers(CommandBufferAllocateInfo).front());
    return CommandBuffer;
}

bufferHandle context::CreateVertexBuffer(f32 *Values, sz Count)
{
    bufferHandle Handle = ResourceManager.Buffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }

    buffer *Buffer = (buffer*)ResourceManager.Buffers.GetResource(Handle);
    
    Buffer->Name = "";
    Buffer->ApiData = new vkBufferData();
    vkBufferData *VkBufferData = (vkBufferData*)Buffer->ApiData;
    

    auto VulkanContext = context::Get();
    
    auto StageBuffer = VulkanContext->GetStageBuffer();
    auto CommandBuffer = VulkanContext->GetCommandBuffer();

    CommandBuffer->Begin();

    auto VertexAllocation = StageBuffer->Submit((uint8_t*)Values, (u32)Count * sizeof(f32));

    Buffer->Init(VertexAllocation._Size, gfx::bufferUsage::VertexBuffer | gfx::bufferUsage::TransferDestination, gfx::memoryUsage::GpuOnly);

    CommandBuffer->CopyBuffer(
        gfx::bufferInfo {StageBuffer->Buffer, VertexAllocation._Offset},
        gfx::bufferInfo {Buffer, 0},
        VertexAllocation._Size
    );
    
    StageBuffer->Flush();
    CommandBuffer->End();

    VulkanContext->SubmitCommandBufferImmediate(CommandBuffer);
    StageBuffer->Reset();     

    return Handle;
}

bufferHandle context::CreateBuffer(sz Size, bufferUsage::Bits Usage, memoryUsage MemoryUsage)
{
    bufferHandle Handle = ResourceManager.Buffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }
    buffer *Buffer = (buffer*)ResourceManager.Buffers.GetResource(Handle);
    Buffer->ApiData = new vkBufferData();
    vkBufferData *VkBufferData = (vkBufferData*)Buffer->ApiData;
    
    constexpr std::array BufferQueueFamilyIndices = {(u32)0};

    //Set size, usage
    Buffer->Size = Size;
    vk::BufferCreateInfo BufferCreateInfo;
    BufferCreateInfo.setSize(Buffer->Size)
                    .setUsage((vk::BufferUsageFlags)Usage)
                    .setSharingMode(vk::SharingMode::eExclusive)
                    .setQueueFamilyIndices(BufferQueueFamilyIndices);

    //Allocate with vma
    VkBufferData->Allocation = gfx::AllocateBuffer(BufferCreateInfo, MemoryUsage, &VkBufferData->Handle);    

    return Handle;
}

stageBuffer context::CreateStageBuffer(sz Size)
{
    stageBuffer Result;
    Result.Init(Size);
    return Result;
}

commandBuffer *context::GetCommandBuffer()
{
    GET_CONTEXT(VkData, this);
    return VkData->ImmediateCommandBuffer;
}

stageBuffer *context::GetStageBuffer()
{
    GET_CONTEXT(VkData, this);
    return &VkData->StageBuffer;
}

void context::SubmitCommandBufferImmediate(commandBuffer *CommandBuffer)
{
    GET_CONTEXT(VkData, this);
    vkCommandBufferData *VkCommandBufferData = (vkCommandBufferData*)CommandBuffer->ApiData;

    vk::SubmitInfo SubmitInfo;
    SubmitInfo.setCommandBuffers(VkCommandBufferData->Handle);
    VkData->DeviceQueue.submit(SubmitInfo, VkData->ImmediateFence);
    auto WaitResult = VkData->Device.waitForFences(VkData->ImmediateFence, false, UINT64_MAX);
    assert(WaitResult == vk::Result::eSuccess);
    VkData->Device.resetFences(VkData->ImmediateFence);
}

}

#endif