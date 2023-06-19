#if API == VK

#include <vulkan/vulkan.hpp>
#include <optional>
#include <iostream>
#include <algorithm>
#include <vk_mem_alloc.h>
#include <glslang/Public/ShaderLang.h>

#include "../Include/GfxContext.h"
#include "../../App/App.h"
#include "Mapping.h"
#include "../Include/Image.h"
#include "VkImage.h"

#define GET_CONTEXT(data, context) \
    vkData *data = (vkData*)context->ApiContextData; \


namespace gfx
{

context* context::Singleton= nullptr;

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
    // commandBuffer ImmediateCommandBuffer{{}};

    vk::Extent2D SurfaceExtent;

    vk::SwapchainKHR Swapchain;
    std::vector<image*> SwapchainImages;
    std::vector<imageUsage::bits> SwapchainImageUsages;
};


context *context::Get()
{
    return Singleton;
}

void CheckRequestedExtensions(context::initializeInfo &InitializeInfos)
{
    InitializeInfos.InfoCallback("Requested Extensions : ");

    auto Extensions = vk::enumerateInstanceExtensionProperties();
    for(const char *ExtensionName : InitializeInfos.Extensions)
    {
        InitializeInfos.InfoCallback("- " + std::string(ExtensionName));

        auto LayerIterator = std::find_if(Extensions.begin(), Extensions.end(), 
            [ExtensionName](const vk::ExtensionProperties &Extension)
            {
                return std::strcmp(Extension.extensionName.data(), ExtensionName) == 0;
            });
        
        if(LayerIterator == Extensions.end())
        {
            InitializeInfos.ErrorCallback("Cannot enable requested extension");
            return;
        }
    }
}


void CheckRequestedLayers(context::initializeInfo &InitializeInfos)
{
    InitializeInfos.InfoCallback("Requested Validation Layers : ");

    auto Extensions = vk::enumerateInstanceLayerProperties();
    for(const char *LayerName : InitializeInfos.Layers)
    {
        InitializeInfos.InfoCallback("- " + std::string(LayerName));

        auto LayerIterator = std::find_if(Extensions.begin(), Extensions.end(), 
            [LayerName](const vk::LayerProperties &Layer)
            {
                return std::strcmp(Layer.layerName.data(), LayerName) == 0;
            });
        
        if(LayerIterator == Extensions.end())
        {
            InitializeInfos.ErrorCallback(("Cannot enable requested layer " + std::string(LayerName)).c_str());
            return;
        }
    }
}

void CreateInstance(context::initializeInfo &InitializeInfo, vkData *VkData)
{
    
    if(InitializeInfo.Debug)  InitializeInfo.Layers = {"VK_LAYER_KHRONOS_validation"};
    vk::ApplicationInfo ApplicationInfo;
    ApplicationInfo.setPApplicationName(InitializeInfo.AppName)
                   .setApplicationVersion(VK_MAKE_VERSION(1,0,0))
                   .setPEngineName(InitializeInfo.EngineName)
                   .setEngineVersion(VK_MAKE_VERSION(1,0,0))
                   .setApiVersion(VK_MAKE_VERSION(InitializeInfo.MajorVersion, InitializeInfo.MinorVersion, 0));

    auto Extensions = InitializeInfo.Extensions;
    Extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    vk::InstanceCreateInfo InstanceCreateInfo;
    InstanceCreateInfo.setPApplicationInfo(&ApplicationInfo)
                      .setPEnabledExtensionNames(Extensions)
                      .setPEnabledLayerNames(InitializeInfo.Layers);

    CheckRequestedExtensions(InitializeInfo);
    CheckRequestedLayers(InitializeInfo);

    VkData->Instance  = vk::createInstance(InstanceCreateInfo);
    VkData->ApiVersion = ApplicationInfo.apiVersion;
    InitializeInfo.InfoCallback("Created Vulkan instance ");
}


bool CheckVulkanPresentationSupport(const vk::Instance& _Instance, const vk::PhysicalDevice& _PhysicalDevice, u32 _FamilyQueueIndex)
{
    return glfwGetPhysicalDevicePresentationSupport(_Instance, _PhysicalDevice, _FamilyQueueIndex) == GLFW_TRUE;
}

std::optional<uint32_t> DetermineQueueFamilyIndex(const vk::Instance &_Instance, const vk::PhysicalDevice &_PhysicalDevice, const vk::SurfaceKHR &_Surface)
{
    auto QueueFamilyProperties = _PhysicalDevice.getQueueFamilyProperties();
    u32 Index=0;

    for(const auto &Property : QueueFamilyProperties)
    {
        if(
            (Property.queueCount > 0 ) &&
            (_PhysicalDevice.getSurfaceSupportKHR(Index, _Surface)) &&
            (CheckVulkanPresentationSupport(_Instance, _PhysicalDevice, Index)) &&
            (Property.queueFlags & vk::QueueFlagBits::eGraphics) &&
            (Property.queueFlags & vk::QueueFlagBits::eCompute)
        )
        {
            return Index;
        }
        Index++;
    }

    return {};
}

static VKAPI_ATTR VkBool32 VKAPI_CALL ValidationLayerCallback(VkDebugUtilsMessageSeverityFlagBitsEXT Severity, VkDebugUtilsMessageTypeFlagsEXT Type, const VkDebugUtilsMessengerCallbackDataEXT *pCallBackData, void *pUserData)
{
    std::cout << pCallBackData->pMessage << std::endl;

    return VK_FALSE;
}

void RecreateSwapchain(u32 Width, u32 Height, vkData *VkData)
{
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
        return;
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
                       .setOldSwapchain(VkData->Swapchain);
    VkData->Swapchain = VkData->Device.createSwapchainKHR(SwapchainCreateInfo);
    
    //Destroy old one
    if(SwapchainCreateInfo.oldSwapchain)
    {
        VkData->Device.destroySwapchainKHR(SwapchainCreateInfo.oldSwapchain);
    }
    auto SwapchainImages = VkData->Device.getSwapchainImagesKHR(VkData->Swapchain);

    //reinitialize internal buffers
    VkData->PresentImageCount = (u32)SwapchainImages.size();
    VkData->SwapchainImages.clear();
    VkData->SwapchainImages.reserve(VkData->PresentImageCount);
    VkData->SwapchainImageUsages.assign(VkData->PresentImageCount, imageUsage::UNKNOWN);

    //Initialize texture representations of swapchain images
    for(u32 i=0; i<VkData->PresentImageCount; i++)
    {
        VkData->SwapchainImages.push_back(
            CreateImage(SwapchainImages[i], VkData->SurfaceExtent.width, VkData->SurfaceExtent.height, FormatFromNative(VkData->SurfaceFormat.format))
        );
    }
}


context* context::Initialize(context::initializeInfo &InitializeInfo, app::window &Window)
{
    if(Singleton==nullptr){
        Singleton = new context();
    }

    Singleton->ApiContextData = new vkData();
    GET_CONTEXT(VkData, Singleton);
    CreateInstance(InitializeInfo, VkData);
    
    
    VkSurfaceKHR Surface;
    (void)glfwCreateWindowSurface(VkData->Instance, Window.GetHandle(), nullptr, &Surface);

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
    DeviceExtensions.push_back(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME);

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
                    .setPNext(&DescriptorIndexingFeatures)
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

    //Initialize swapchain
    RecreateSwapchain(SurfaceCapabilities.maxImageExtent.width, SurfaceCapabilities.maxImageExtent.height, VkData);
    InitializeInfo.InfoCallback("Created Swapchain");

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
    
    // VkData->ImmediateCommandBuffer = commandBuffer(VkData->Device.allocateCommandBuffers(CommandBufferAllocateInfo).front());
    // InitializeInfo.InfoCallback("Created Command Buffer");

    // //Initialize descriptor cache
    // VkData->DescriptorCache.Init();
    
    // //Initialize virtual frames
    // VkData->VirtualFrames.Init(InitializeInfo.VirtualFrameCount, InitializeInfo.MaxStageBufferSize);

    InitializeInfo.InfoCallback("Initialization Finished");    

    return Singleton;
}
}

#endif