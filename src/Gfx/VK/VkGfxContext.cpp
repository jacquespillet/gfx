#if GFX_API == GFX_VK

#include <vulkan/vulkan.hpp>
#include <optional>
#include <iostream>
#include <memory>
#include <algorithm>
#include <glslang/Public/ShaderLang.h>

#include "../../App/App.h"
#include "../Include/GfxContext.h"
#include "../Include/Image.h"
#include "../Include/CommandBuffer.h"
#include "../Include/Swapchain.h"
#include "../Include/Buffer.h"
#include "../Include/Pipeline.h"
#include "../Include/Shader.h"
#include "../Include/RenderPass.h"
#include "../Include/Framebuffer.h"
#include "../Include/Memory.h"
#include "../Common/Util.h"
#include "VkMapping.h"
#include "VkImage.h"
#include "VkCommandBuffer.h"
#include "VkGfxContext.h"
#include "VkSwapchain.h"
#include "VkBuffer.h"
#include "VkShader.h"
#include "VkUniform.h"
#include "VkMemoryAllocation.h"
#include "VkUtil.h"
#include "VkRenderPass.h"
#include "VkCommon.h"
#include "VkPipeline.h"
#include "VkResourceManager.h"
#include "VkFramebuffer.h"


namespace gfx
{

std::shared_ptr<context> context::Singleton = {};

context *context::Get()
{
    return Singleton.get();
}


bool GetSupportedDepthFormat(vk::PhysicalDevice PhysicalDevice, format *DepthFormat)
{
    std::vector<format> DepthFormats = {
        format::D32_SFLOAT_S8_UINT,
        format::D32_SFLOAT,
        format::D24_UNORM_S8_UINT,
        format::D16_UNORM_S8_UINT,
        format::D16_UNORM_S8_UINT
    };

    for(auto &Format : DepthFormats)
    {
        vk::FormatProperties FormatProperties = PhysicalDevice.getFormatProperties(FormatToNative(Format));
        
        if(FormatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment)
        {
            *DepthFormat = Format;
            return true;
        }
    }
    return false;
}

void CreateSwapchainFramebuffer(std::shared_ptr<image> *ColorImages, std::shared_ptr<image> DepthStencilImage, renderPassHandle RenderPassHandle, sz ImagesCount, vkSwapchainData *VkSwapchainData)
{
    auto Context = context::Get();
    GET_CONTEXT(VkData, Context);

    int Width = ColorImages[0]->Extent.Width;
    int Height = ColorImages[0]->Extent.Height;

    renderPass *RenderPass = (renderPass*) Context->ResourceManager.RenderPasses.GetResource(RenderPassHandle);
    vk::RenderPass VkRenderPass = std::static_pointer_cast<vkRenderPassData>(RenderPass->ApiData)->NativeHandle;
     
    for (size_t i = 0; i < ImagesCount; i++)
    {
        framebufferHandle FramebufferHandle = Context->ResourceManager.Framebuffers.ObtainResource();
        if(FramebufferHandle == InvalidHandle)
        {
            assert(false);
            return;
        }
        framebuffer *Framebuffer = (framebuffer*)Context->ResourceManager.Framebuffers.GetResource(FramebufferHandle);
        Framebuffer->Width = ColorImages[i]->Extent.Width;
        Framebuffer->Height = ColorImages[i]->Extent.Height;
        Framebuffer->ApiData = std::make_shared<vkFramebufferData>();
        std::shared_ptr<vkFramebufferData> VkFramebufferData = std::static_pointer_cast<vkFramebufferData>(Framebuffer->ApiData);
        if(VkData->MultisamplingEnabled) VkFramebufferData->IsMultiSampled=true;

        std::shared_ptr<vkImageData> VkColorImage = std::static_pointer_cast<vkImageData>(ColorImages[i]->ApiData);
        std::shared_ptr<vkImageData> VkDepthImage = std::static_pointer_cast<vkImageData>(DepthStencilImage->ApiData);

        std::vector<vk::ImageView> Attachments;
        if(VkFramebufferData->IsMultiSampled)
        {

            VkFramebufferData->MultiSampledColorImage = std::make_shared<image>();
            VkFramebufferData->MultiSampledColorImage->Init(Framebuffer->Width, Framebuffer->Height, ColorImages[i]->Format, imageUsage::COLOR_ATTACHMENT, memoryUsage::GpuOnly, context::Get()->MultiSampleCount);
            VkFramebufferData->MultiSampledDepthStencilImage = std::make_shared<image>();
            VkFramebufferData->MultiSampledDepthStencilImage->Init(Framebuffer->Width, Framebuffer->Height, DepthStencilImage->Format, imageUsage::DEPTH_STENCIL_ATTACHMENT, memoryUsage::GpuOnly, context::Get()->MultiSampleCount);
                    
            std::shared_ptr<vkImageData> VkMultisampledColorImage = std::static_pointer_cast<vkImageData>(VkFramebufferData->MultiSampledColorImage->ApiData);
            std::shared_ptr<vkImageData> VkMultisampledDepthStencilImage = std::static_pointer_cast<vkImageData>(VkFramebufferData->MultiSampledDepthStencilImage->ApiData);
            Attachments = 
            {
                VkMultisampledColorImage->DefaultImageViews.NativeView,
                VkColorImage->DefaultImageViews.NativeView,
                VkMultisampledDepthStencilImage->DefaultImageViews.NativeView,
            };
        }
        else
        {
            Attachments = 
            {
                VkColorImage->DefaultImageViews.NativeView,
                VkDepthImage->DefaultImageViews.NativeView
            };
        }  


        vk::FramebufferCreateInfo FramebufferCreateInfo;
        FramebufferCreateInfo.setRenderPass(VkRenderPass)
                            .setAttachments(Attachments)
                            .setWidth(ColorImages[i]->Extent.Width)
                            .setHeight(ColorImages[i]->Extent.Height)
                            .setLayers(1);
        Framebuffer->RenderPass = RenderPassHandle;
        VkFramebufferData->DepthStencilImage = DepthStencilImage;
        VkFramebufferData->ColorImages = ColorImages;
        VkFramebufferData->Handle = VkData->Device.createFramebuffer(FramebufferCreateInfo);

        VkSwapchainData->Framebuffers[i] = FramebufferHandle;
    }
}

std::shared_ptr<swapchain> context::CreateSwapchain(u32 Width, u32 Height, std::shared_ptr<swapchain> OldSwapchain)
{
    GET_CONTEXT(VkData, this);
    b8 Create = OldSwapchain == nullptr;
    b8 Recreate = !Create;

    if(Create)
    {
        Swapchain = std::make_shared<swapchain>();
        Swapchain->ApiData = std::make_shared<vkSwapchainData>();
    }
    std::shared_ptr<vkSwapchainData> VkSwapchainData = std::static_pointer_cast<vkSwapchainData>(Swapchain->ApiData);

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

    VkSwapchainData->ImageCount = VkData->PresentImageCount;
    
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
    if(Recreate) SwapchainCreateInfo.setOldSwapchain(VkSwapchainData->Handle);
    VkSwapchainData->Handle = VkData->Device.createSwapchainKHR(SwapchainCreateInfo);
    
    //Destroy old one
    if(Recreate)
    {
        VkData->Device.destroySwapchainKHR(SwapchainCreateInfo.oldSwapchain);
        for(u32 i=0; i<VkData->PresentImageCount; i++)
        {
            framebuffer *Framebuffer = (framebuffer *) ResourceManager.Framebuffers.GetResource(VkSwapchainData->Framebuffers[i]);
            std::shared_ptr<vkFramebufferData> VkFramebufferData = std::static_pointer_cast<vkFramebufferData>(Framebuffer->ApiData);
            VkData->Device.destroyFramebuffer(VkFramebufferData->Handle);
            ResourceManager.Framebuffers.ReleaseResource(VkSwapchainData->Framebuffers[i]);
            
            std::shared_ptr<vkImageData> VkImageData = std::static_pointer_cast<vkImageData>(VkSwapchainData->SwapchainImages[i]->ApiData);
            VkData->Device.destroyImageView(VkImageData->DefaultImageViews.NativeView);
            
            //Destroy the multisampled render targets
            if(VkFramebufferData->IsMultiSampled)
            {
                VkFramebufferData->MultiSampledColorImage->Destroy();
                VkFramebufferData->MultiSampledDepthStencilImage->Destroy();
            }

            if(i==0) 
            {
                VkFramebufferData->DepthStencilImage->Destroy();        
                
            }        
        }          
    }
    auto SwapchainImages = VkData->Device.getSwapchainImagesKHR(VkSwapchainData->Handle);

    //reinitialize internal buffers
    VkData->PresentImageCount = (u32)SwapchainImages.size();
    for (size_t i = 0; i < VkData->PresentImageCount; i++)
    {
        VkSwapchainData->SwapchainImageUsages[i] = imageUsage::UNKNOWN;
    }

    renderPassHandle SwapchainPassHandle = GetDefaultRenderPass();
    
    //Initialize texture representations of swapchain images
    for(u32 i=0; i<VkData->PresentImageCount; i++)
    {
        VkSwapchainData->SwapchainImages[i] = 
            std::make_shared<image>(SwapchainImages[i], VkData->SurfaceExtent.width, VkData->SurfaceExtent.height, FormatFromNative(VkData->SurfaceFormat.format));
    }


    format DepthFormat;
    bool FoundDepthFormat = GetSupportedDepthFormat(VkData->PhysicalDevice, &DepthFormat);
    std::shared_ptr<image> DepthStencil = std::make_shared<image>();
    DepthStencil->Init(VkData->SurfaceExtent.width, VkData->SurfaceExtent.height, DepthFormat, imageUsage::DEPTH_STENCIL_ATTACHMENT, memoryUsage::GpuOnly);

    CreateSwapchainFramebuffer(VkSwapchainData->SwapchainImages, DepthStencil, SwapchainPassHandle, VkData->PresentImageCount, VkSwapchainData.get());


    return Swapchain;
}

std::shared_ptr<context> context::Initialize(context::initializeInfo &InitializeInfo, app::window &Window)
{
    if(Singleton==nullptr){
        Singleton = std::make_shared<context>();
    }

    Singleton->ResourceManager.Init();

    Singleton->ApiContextData = std::make_shared<vkData>();
    Singleton->Window = &Window;
    
    GET_CONTEXT(VkData, Singleton);
    CreateInstance(InitializeInfo, VkData.get());
    VkData->MultisamplingEnabled = InitializeInfo.EnableMultisampling;
    Singleton->MultiSampleCount = (InitializeInfo.EnableMultisampling) ? 4 : 1;

      
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

        vk::SampleCountFlags Counts = Properties.limits.framebufferColorSampleCounts & Properties.limits.framebufferDepthSampleCounts;
        if(InitializeInfo.EnableMultisampling && !(Counts & vk::SampleCountFlagBits::e4))
        {
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
    Singleton->SwapchainOutput.Reset();
    for(const auto &SurfaceFormat : SurfaceFormats)
    {
        if(SurfaceFormat.format == vk::Format::eR8G8B8A8Unorm || SurfaceFormat.format == vk::Format::eB8G8R8A8Unorm)
        {
            VkData->SurfaceFormat = SurfaceFormat;
        }
    }
    
    format DepthFormat; GetSupportedDepthFormat(VkData->PhysicalDevice, &DepthFormat);
    Singleton->SwapchainOutput.Color(FormatFromNative(VkData->SurfaceFormat.format), imageLayout::PresentSrcKHR, renderPassOperation::Clear);
    Singleton->SwapchainOutput.Depth(DepthFormat, imageLayout::DepthStencilAttachmentOptimal);
    Singleton->SwapchainOutput.SetDepthStencilOperation(renderPassOperation::Clear, renderPassOperation::Clear);
    Singleton->SwapchainOutput.Name = AllocateCString("Swapchain");
    Singleton->SwapchainOutput.SampleCount = InitializeInfo.EnableMultisampling ? 4 : 1;
    
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


    vk::DeviceCreateInfo DeviceCreateInfo = {};
    DeviceCreateInfo.setQueueCreateInfos(DeviceQueueCreateInfo)
                    .setPEnabledExtensionNames(DeviceExtensions)
                    //.setPNext(&DescriptorIndexingFeatures)
                    ;

    VkData->Device = VkData->PhysicalDevice.createDevice(DeviceCreateInfo);
    VkData->DeviceQueue = VkData->Device.getQueue(VkData->QueueFamilyIndex, 0);

    InitializeInfo.InfoCallback("Created device and queues");


    VkData->GetRenderPass(Singleton->SwapchainOutput, "Swapchain");

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

    //Descriptor pool
    std::array DescriptorPoolSizes = 
    {
        vk::DescriptorPoolSize (vk::DescriptorType::eSampler, 1024),
        vk::DescriptorPoolSize (vk::DescriptorType::eCombinedImageSampler, 1024),
        vk::DescriptorPoolSize (vk::DescriptorType::eSampledImage, 1024),
        vk::DescriptorPoolSize (vk::DescriptorType::eStorageImage, 1024),
        vk::DescriptorPoolSize (vk::DescriptorType::eUniformTexelBuffer, 1024),
        vk::DescriptorPoolSize (vk::DescriptorType::eStorageTexelBuffer, 1024),
        vk::DescriptorPoolSize (vk::DescriptorType::eUniformBuffer, 1024),
        vk::DescriptorPoolSize (vk::DescriptorType::eStorageBuffer, 1024),
        vk::DescriptorPoolSize (vk::DescriptorType::eUniformBufferDynamic, 1024),
        vk::DescriptorPoolSize (vk::DescriptorType::eStorageBufferDynamic, 1024),
        vk::DescriptorPoolSize (vk::DescriptorType::eInputAttachment, 1024),
    };
    vk::DescriptorPoolCreateInfo DescriptorPoolCreateInfo;
    DescriptorPoolCreateInfo.setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet | vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind)
                            .setPoolSizes(DescriptorPoolSizes)
                            .setMaxSets(2048 * (u32) DescriptorPoolSizes.size());
    VkData->DescriptorPool = VkData->Device.createDescriptorPool(DescriptorPoolCreateInfo);    

    
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
    VkData->VirtualFrames.Init(InitializeInfo.VirtualFrameCount, InitializeInfo.MaxStageBufferSize);
    InitializeInfo.InfoCallback("Initialization Finished");    


    return Singleton;
}

std::shared_ptr<commandBuffer> context::CreateCommandBuffer()
{
    GET_CONTEXT(VkData, this);
    vk::CommandBufferAllocateInfo CommandBufferAllocateInfo;
    CommandBufferAllocateInfo.setLevel(vk::CommandBufferLevel::ePrimary)
                             .setCommandPool(VkData->CommandPool)
                             .setCommandBufferCount(1);
    
    std::shared_ptr<commandBuffer> CommandBuffer = CreateVkCommandBuffer(VkData->Device.allocateCommandBuffers(CommandBufferAllocateInfo).front());
    return CommandBuffer;
}

void context::OnResize(u32 NewWidth, u32 NewHeight)
{
    CreateSwapchain(NewWidth, NewHeight, Swapchain);
}


bufferHandle CreateVertexBufferStream(f32 *Values, sz Count, sz Stride, const std::vector<vertexInputAttribute> &Attributes)
{
    context *Context = context::Get();

    bufferHandle Handle = Context->ResourceManager.Buffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }

    buffer *Buffer = (buffer*)Context->ResourceManager.Buffers.GetResource(Handle);
    
    Buffer->Name = "";
    Buffer->ApiData = std::make_shared<vkBufferData>();
    std::shared_ptr<vkBufferData> VkBufferData = std::static_pointer_cast<vkBufferData>(Buffer->ApiData);
    *VkBufferData = vkBufferData();

    auto VulkanContext = context::Get();
    
    auto StageBuffer = VulkanContext->GetStageBuffer();
    auto CommandBuffer = VulkanContext->GetImmediateCommandBuffer();

    CommandBuffer->Begin();

    auto VertexAllocation = StageBuffer->Submit((uint8_t*)Values, (u32)Count * sizeof(f32));

    Buffer->Init(VertexAllocation.Size, gfx::bufferUsage::VertexBuffer, gfx::memoryUsage::GpuOnly);
  
    CommandBuffer->CopyBuffer(
        gfx::bufferInfo {StageBuffer->GetBuffer(), VertexAllocation.Offset},
        gfx::bufferInfo {Buffer, 0},
        VertexAllocation.Size
    );
    
    StageBuffer->Flush();
    CommandBuffer->End();

    VulkanContext->SubmitCommandBufferImmediate(CommandBuffer);
    StageBuffer->Reset();     

    return Handle;
}

vertexBufferHandle context::CreateVertexBuffer(const vertexBufferCreateInfo &CreateInfo)
{
    GET_CONTEXT(VkData, context::Get());
    
    vertexBufferHandle Handle = this->ResourceManager.VertexBuffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        assert(false);
        return Handle;
    }
    vertexBuffer *VertexBuffer = (vertexBuffer*) this->ResourceManager.VertexBuffers.GetResource(Handle);
    VertexBuffer->NumVertexStreams = CreateInfo.NumVertexStreams;
    memcpy(&VertexBuffer->VertexStreams[0], &CreateInfo.VertexStreams[0], commonConstants::MaxVertexStreams * sizeof(vertexStreamData));
    
    for(sz i=0; i<VertexBuffer->NumVertexStreams; i++)
    {
        std::vector<vertexInputAttribute> Attributes(VertexBuffer->VertexStreams[i].AttributesCount);
        memcpy(&Attributes[0], &CreateInfo.VertexStreams[i].InputAttributes, VertexBuffer->VertexStreams[i].AttributesCount * sizeof(vertexInputAttribute));

        VertexBuffer->VertexStreams[i].Buffer = CreateVertexBufferStream((f32*)VertexBuffer->VertexStreams[i].Data, VertexBuffer->VertexStreams[i].Size, VertexBuffer->VertexStreams[i].Stride, Attributes);
    }
    
    return Handle;
}

bufferHandle context::CreateBuffer(sz Size, bufferUsage::value Usage, memoryUsage MemoryUsage)
{
    bufferHandle Handle = ResourceManager.Buffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }
    buffer *Buffer = (buffer*)ResourceManager.Buffers.GetResource(Handle);
    Buffer->ApiData = std::make_shared<vkBufferData>();
    std::shared_ptr<vkBufferData> VkBufferData = std::static_pointer_cast<vkBufferData>(Buffer->ApiData);
    *VkBufferData = vkBufferData(); 

    //TODO: Use buffer->Init() here!
    
    //We assume that if the buffer is index or vertex, we'll copy to it
    if(Usage & bufferUsage::IndexBuffer || Usage & bufferUsage::VertexBuffer)
        Usage |= bufferUsage::TransferDestination;
    
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

imageHandle context::CreateImage(const imageData &ImageData, const imageCreateInfo& CreateInfo)
{
    imageHandle ImageHandle = ResourceManager.Images.ObtainResource();
    if(ImageHandle == InvalidHandle)
    {
        return ImageHandle;
    }
    image *Image = (image*)ResourceManager.Images.GetResource(ImageHandle);
    *Image = image();
    Image->Init(ImageData, CreateInfo);
    return ImageHandle;
}

stageBuffer context::CreateStageBuffer(sz Size)
{
    stageBuffer Result;
    Result.Init(Size);
    return Result;
}

void context::StartFrame()
{
    GET_CONTEXT(VkData, this);
    VkData->VirtualFrames.StartFrame();
}

void context::EndFrame()
{
    GET_CONTEXT(VkData, this);
    VkData->VirtualFrames.EndFrame();
}

void context::Present()
{
    GET_CONTEXT(VkData, this);
    VkData->VirtualFrames.Present();
}

commandBuffer *context::GetImmediateCommandBuffer()
{
    GET_CONTEXT(VkData, this);
    return VkData->ImmediateCommandBuffer.get();
}

std::shared_ptr<commandBuffer> context::GetCurrentFrameCommandBuffer()
{
    GET_CONTEXT(VkData, this);
    return VkData->VirtualFrames.GetCurrentFrame().Commands;
}

stageBuffer *context::GetStageBuffer()
{
    GET_CONTEXT(VkData, this);
    return &VkData->StageBuffer;
}

void context::SubmitCommandBufferImmediate(commandBuffer *CommandBuffer)
{
    GET_CONTEXT(VkData, this);
    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(CommandBuffer->ApiData);

    vk::SubmitInfo SubmitInfo;
    SubmitInfo.setCommandBuffers(VkCommandBufferData->Handle);
    VkData->DeviceQueue.submit(SubmitInfo, VkData->ImmediateFence);
    auto WaitResult = VkData->Device.waitForFences(VkData->ImmediateFence, false, UINT64_MAX);
    assert(WaitResult == vk::Result::eSuccess);
    VkData->Device.resetFences(VkData->ImmediateFence);
}

void context::SubmitCommandBuffer(commandBuffer *CommandBuffer)
{
    GET_CONTEXT(VkData, this);
    std::shared_ptr<vkCommandBufferData> VkCommandBufferData = std::static_pointer_cast<vkCommandBufferData>(CommandBuffer->ApiData);

    vk::SubmitInfo SubmitInfo;
    SubmitInfo.setCommandBuffers(VkCommandBufferData->Handle);
    VkData->DeviceQueue.submit(SubmitInfo, VkData->ImmediateFence);
    auto WaitResult = VkData->Device.waitForFences(VkData->ImmediateFence, false, UINT64_MAX);
    assert(WaitResult == vk::Result::eSuccess);
    VkData->Device.resetFences(VkData->ImmediateFence);
}


shaderStateHandle CreateShaderState(const shaderStateCreation &Creation)
{
    shaderStateHandle Handle = InvalidHandle;
    if(Creation.StagesCount == 0 || Creation.Stages == nullptr) 
    {
        printf("Error creating shader state\n");
        return Handle;
    }
    context *Context = context::Get();
    GET_CONTEXT(VkData, Context);

    Handle = Context->ResourceManager.Shaders.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }

    u32 CompiledShaders=0;
    
    shader *ShaderState = (shader*)Context->ResourceManager.Shaders.GetResource(Handle);
    ShaderState->ApiData = std::make_shared<vkShaderData>();
    std::shared_ptr<vkShaderData> VkShaderData = std::static_pointer_cast<vkShaderData>(ShaderState->ApiData);

    ShaderState->GraphicsPipeline = true;
    ShaderState->ActiveShaders=0;
    VkShaderData->SpirvParseResults= {};
    for(CompiledShaders = 0; CompiledShaders < Creation.StagesCount; CompiledShaders++)
    {
        const shaderStage &ShaderStage = Creation.Stages[CompiledShaders];
        if(ShaderStage.Stage == shaderStageFlags::Compute)
        {
            ShaderState->GraphicsPipeline=false;
        }

        vk::ShaderModuleCreateInfo ShaderCreateInfo;
        if(Creation.SpvInput)
        {
            ShaderCreateInfo.codeSize = ShaderStage.CodeSize;
            ShaderCreateInfo.pCode = reinterpret_cast< const u32* >(ShaderStage.Code);
        }
        else
        {
            ShaderCreateInfo = CompileShader(ShaderStage.Code, ShaderStage.CodeSize, ShaderStage.Stage, Creation.Name);
        }


        vk::PipelineShaderStageCreateInfo &ShaderStageCreateInfo = VkShaderData->ShaderStageCreateInfo[CompiledShaders];
        memset(&ShaderStageCreateInfo, 0, sizeof(vk::PipelineShaderStageCreateInfo));
        ShaderStageCreateInfo = vk::PipelineShaderStageCreateInfo();
        ShaderStageCreateInfo.setPName("main") .setStage(ShaderStageToNative(ShaderStage.Stage));

        VkShaderData->ShaderStageCreateInfo[CompiledShaders].module = VkData->Device.createShaderModule(ShaderCreateInfo);

        ParseSpirv((void*)((char*)ShaderCreateInfo.pCode), ShaderCreateInfo.codeSize, VkShaderData->SpirvParseResults);

        DeallocateMemory((void*)ShaderCreateInfo.pCode);
    }

    ShaderState->ActiveShaders = CompiledShaders;
    ShaderState->Name = Creation.Name;

    return Handle;
}


descriptorSetLayoutHandle CreateDescriptorSetLayout(const descriptorSetLayoutCreation &Creation)
{
    context *Context = context::Get();
    GET_CONTEXT(VkData, Context);

    std::shared_ptr<vkResourceManagerData> VkResourceManager = std::static_pointer_cast<vkResourceManagerData>(Context->ResourceManager.ApiData);

    descriptorSetLayoutHandle Handle = VkResourceManager->DescriptorSetLayouts.ObtainResource(); 
    if(Handle == InvalidHandle)
    {
        return Handle;
    }

    descriptorSetLayout *DescriptorSetLayout = (descriptorSetLayout *)VkResourceManager->DescriptorSetLayouts.GetResource(Handle);

    memset(&DescriptorSetLayout->UsedBindings[0], 0, vkConstants::MaxDescriptorsPerSet * sizeof(b8));

    u16 MaxBinding=0;
    for(u32 i=0; i<Creation.NumBindings; i++)
    {
        const descriptorSetLayoutCreation::binding &Binding = Creation.Bindings[i];
        MaxBinding = std::max(MaxBinding, Binding.Start);
    }    
    MaxBinding +=1;

    DescriptorSetLayout->BindingCount = (u16)Creation.NumBindings;
    u8 *Memory = (u8*)AllocateMemory((sizeof(vk::DescriptorSetLayoutBinding) + sizeof(descriptorBinding)) * Creation.NumBindings + (sizeof(u8) * MaxBinding));
    DescriptorSetLayout->Bindings = (descriptorBinding*)Memory;
    DescriptorSetLayout->BindingNativeHandle = (vk::DescriptorSetLayoutBinding*)(Memory + sizeof(descriptorBinding) * Creation.NumBindings);
    DescriptorSetLayout->IndexToBinding = (u8*)(DescriptorSetLayout->BindingNativeHandle + Creation.NumBindings); 
    DescriptorSetLayout->Handle = Handle;
    DescriptorSetLayout->SetIndex = u16(Creation.SetIndex);
    DescriptorSetLayout->Bindless = Creation.Bindless ? 1 : 0;



    u32 UsedBindings = 0;
    for(u32 R = 0; R < Creation.NumBindings; ++R)
    {
        descriptorBinding &Binding = DescriptorSetLayout->Bindings[R];
        const descriptorSetLayoutCreation::binding &InputBinding = Creation.Bindings[R];
        Binding.Start = InputBinding.Start == UINT16_MAX ? (u16)R : InputBinding.Start;
        Binding.Count = InputBinding.Count;
        Binding.Type = InputBinding.Type;
        Binding.Name = InputBinding.Name;

        DescriptorSetLayout->IndexToBinding[Binding.Start] = R;

        vk::DescriptorSetLayoutBinding &VkBinding = DescriptorSetLayout->BindingNativeHandle[UsedBindings];
        ++UsedBindings;

        VkBinding.binding = Binding.Start;
        VkBinding.descriptorType = InputBinding.Type;
        VkBinding.descriptorCount = InputBinding.Count;
        VkBinding.stageFlags = vk::ShaderStageFlagBits::eAll;
        VkBinding.pImmutableSamplers = nullptr; 

        DescriptorSetLayout->UsedBindings[Binding.Start] = true;
    }

    vk::DescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo;
    DescriptorSetLayoutCreateInfo.setBindingCount(UsedBindings)
                                 .setPBindings(DescriptorSetLayout->BindingNativeHandle);

    
    DescriptorSetLayout->NativeHandle = VkData->Device.createDescriptorSetLayout(DescriptorSetLayoutCreateInfo);
    

    return Handle;
}

vk::RenderPass CreateRenderPass(vkData *VkData, const renderPassOutput &Output)
{

    vk::AttachmentDescription ColorAttachments[16] = {};
    vk::AttachmentReference ColorAttachmentsRefs[commonConstants::MaxImageOutputs] = {};
    vk::AttachmentReference ColorAttachmentsResolveRefs[commonConstants::MaxImageOutputs] = {};

    vk::AttachmentLoadOp DepthOp, StencilOp;
    vk::ImageLayout DepthInitialLayout;

    switch (Output.DepthOperation)
    {
    case renderPassOperation::Load:
        DepthOp= vk::AttachmentLoadOp::eLoad;
        DepthInitialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        break;
    case renderPassOperation::Clear:
        DepthOp= vk::AttachmentLoadOp::eClear;
        DepthInitialLayout = vk::ImageLayout::eUndefined;
        break;
    default:
        DepthOp= vk::AttachmentLoadOp::eDontCare;
        DepthInitialLayout = vk::ImageLayout::eUndefined;
        break;
    }

    switch (Output.StencilOperation)
    {
    case renderPassOperation::Load:
        StencilOp= vk::AttachmentLoadOp::eLoad;
        break;
    case renderPassOperation::Clear:
        StencilOp= vk::AttachmentLoadOp::eClear;
        break;
    default:
        StencilOp= vk::AttachmentLoadOp::eDontCare;
        break;
    }

    //Create the color attachment descriptions and references
    u32 C = 0;
    for(; C < Output.NumColorFormats; ++C)
    {
        vk::AttachmentLoadOp ColorLoadOp;
        vk::ImageLayout ColorInitialLayout;

        switch (Output.ColorOperations[C])
        {
        case renderPassOperation::Load:
            ColorLoadOp= vk::AttachmentLoadOp::eLoad;
            ColorInitialLayout = vk::ImageLayout::eColorAttachmentOptimal;
            break;
        case renderPassOperation::Clear:
            ColorLoadOp= vk::AttachmentLoadOp::eClear;
            ColorInitialLayout = vk::ImageLayout::eUndefined;
            break;
        default:
            ColorLoadOp= vk::AttachmentLoadOp::eDontCare;
            ColorInitialLayout = vk::ImageLayout::eUndefined;
            break;
        }

        vk::AttachmentDescription &ColorAttachment = ColorAttachments[C];
        ColorAttachment.setFormat(FormatToNative(Output.ColorFormats[C]))
                       .setSamples(SampleCountToNative(Output.SampleCount))
                       .setLoadOp(ColorLoadOp)
                       .setStoreOp(vk::AttachmentStoreOp::eStore)
                       .setStencilLoadOp(StencilOp)
                       .setStencilStoreOp(vk::AttachmentStoreOp::eStore)
                       .setInitialLayout(ColorInitialLayout)
                       .setFinalLayout(Output.SampleCount > 1 ? vk::ImageLayout::eColorAttachmentOptimal : ImageLayoutToNative(Output.ColorFinalLayouts[C]));

        vk::AttachmentReference &Ref = ColorAttachmentsRefs[C];
        Ref.attachment = C;
        Ref.layout = vk::ImageLayout::eColorAttachmentOptimal;

        if(Output.SampleCount>1)
        {
            vk::AttachmentDescription &ColorAttachment = ColorAttachments[C + Output.NumColorFormats];
            ColorAttachment.setFormat(FormatToNative(Output.ColorFormats[C]))
                        .setSamples(vk::SampleCountFlagBits::e1)
                        .setLoadOp(ColorLoadOp)
                        .setStoreOp(vk::AttachmentStoreOp::eStore)
                        .setStencilLoadOp(StencilOp)
                        .setStencilStoreOp(vk::AttachmentStoreOp::eStore)
                        .setInitialLayout(ColorInitialLayout)
                        .setFinalLayout(ImageLayoutToNative(Output.ColorFinalLayouts[C]));        
        
            vk::AttachmentReference &Ref = ColorAttachmentsResolveRefs[C];
            Ref.attachment = C + Output.NumColorFormats;
            Ref.layout = vk::ImageLayout::eColorAttachmentOptimal;
        }

    }

    //Create the depth attachment description
    vk::AttachmentDescription DepthAttachment{};
    vk::AttachmentReference DepthRef{};
    if(Output.DepthStencilFormat != format::UNDEFINED)
    {
        DepthAttachment.setFormat(FormatToNative(Output.DepthStencilFormat))
                        .setSamples(SampleCountToNative(Output.SampleCount))
                        .setLoadOp(DepthOp)
                        .setStoreOp(vk::AttachmentStoreOp::eStore)
                        .setStencilLoadOp(StencilOp)
                        .setStencilStoreOp(vk::AttachmentStoreOp::eStore)
                        .setInitialLayout(DepthInitialLayout)
                        .setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

        DepthRef.attachment = Output.SampleCount > 1 ? Output.NumColorFormats * 2 : C;
        DepthRef.layout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }

    vk::SubpassDescription Subpass = {};
    Subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
    Subpass.setColorAttachmentCount(Output.NumColorFormats)
           .setPColorAttachments(ColorAttachmentsRefs)
           .setPResolveAttachments(Output.SampleCount > 1 ?  ColorAttachmentsResolveRefs : nullptr)
           .setPDepthStencilAttachment(nullptr);

    vk::AttachmentDescription Attachments[commonConstants::MaxImageOutputs * 2 + 1]{};
    u32 LastColorAttachment = Output.SampleCount > 1 ? Output.NumColorFormats * 2 : Output.NumColorFormats;

    for(u32 ActiveAttachments=0; ActiveAttachments < LastColorAttachment; ++ActiveAttachments)
    {
        Attachments[ActiveAttachments] = ColorAttachments[ActiveAttachments];
    }
    
    u32 DepthStencilCount=0;
    if(Output.DepthStencilFormat != format::UNDEFINED)
    {
        Attachments[LastColorAttachment] = DepthAttachment;
        Subpass.setPDepthStencilAttachment(&DepthRef);
        DepthStencilCount=1;
    }

    vk::RenderPassCreateInfo RenderPassCreateInfo;
    RenderPassCreateInfo.setAttachmentCount(Output.NumColorFormats + DepthStencilCount + (Output.SampleCount > 1 ? Output.NumColorFormats : 0))
                        .setPAttachments(Attachments)
                        .setSubpassCount(1)
                        .setPSubpasses(&Subpass);
    
    vk::RenderPass RenderPass = VkData->Device.createRenderPass(RenderPassCreateInfo);

    return RenderPass;    
}

renderPass *vkData::GetRenderPass(const renderPassOutput &Output, std::string Name)
{
    if(RenderPassCache.find(Name) != RenderPassCache.end())
    {
        renderPass *RenderPass = (renderPass*) context::Get()->ResourceManager.RenderPasses.GetResource(RenderPassCache[Name]);
        return RenderPass;
    }

    renderPassHandle RenderPassHandle = context::Get()->ResourceManager.RenderPasses.ObtainResource();
    if(RenderPassHandle == InvalidHandle)
    {
        assert(false);
        return {};
    }
    
    renderPass *RenderPass = (renderPass*) context::Get()->ResourceManager.RenderPasses.GetResource(RenderPassHandle);
    RenderPass->Name = Name;
    RenderPass->ApiData = std::make_shared<vkRenderPassData>();
    std::shared_ptr<vkRenderPassData> VkRenderPassData = std::static_pointer_cast<vkRenderPassData>(RenderPass->ApiData);
    VkRenderPassData->NativeHandle =  CreateRenderPass(this, Output);


    RenderPassCache[Name] = RenderPassHandle;
    return RenderPass;
}

pipelineHandle context::CreatePipeline(const pipelineCreation &PipelineCreation)
{
    GET_CONTEXT(VkData, this);

//Pipeline cache
#if 0
    vk::PipelineCache PipelineCache;
    vk::PipelineCacheCreateInfo PipelineCacheCreateInfo;

    const char *PipelineCachePath = PipelineCreation.Name;
    b8 CacheExists = FileExists(PipelineCachePath);
    if(PipelineCachePath != nullptr && CacheExists)
    {
        fileContent PipelineCacheFile = ReadFileBinary(PipelineCachePath);
        vk::PipelineCacheHeaderVersionOne *CacheHeader = (vk::PipelineCacheHeaderVersionOne*)PipelineCacheFile.Data;

        if(CacheHeader->deviceID == VkData->PhysicalDeviceProperties.deviceID && 
           CacheHeader->vendorID == VkData->PhysicalDeviceProperties.vendorID &&
           memcmp(CacheHeader->pipelineCacheUUID, VkData->PhysicalDeviceProperties.pipelineCacheUUID, VK_UUID_SIZE) == 0)
        {
            PipelineCacheCreateInfo.setInitialDataSize(PipelineCacheFile.Size).setPInitialData(PipelineCacheFile.Data);
        }
        else
        {
            CacheExists=false;
        }

        PipelineCache = VkData->Device.createPipelineCache(PipelineCacheCreateInfo);
        // delete PipelineCacheFile.Data;
    }
    else
    {
        PipelineCache = VkData->Device.createPipelineCache(PipelineCacheCreateInfo);
    }
#endif

    pipelineHandle Handle = ResourceManager.Pipelines.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }


    shaderStateHandle ShaderState = CreateShaderState(PipelineCreation.Shaders);
    if(ShaderState == InvalidHandle)
    {
        ResourceManager.Pipelines.ReleaseResource(Handle);
        Handle = InvalidHandle;
        return Handle;
    }

    pipeline *Pipeline = (pipeline*)ResourceManager.Pipelines.GetResource(Handle);
    Pipeline->ApiData = std::make_shared<vkPipelineData>();
    std::shared_ptr<vkPipelineData> VkPipelineData = std::static_pointer_cast<vkPipelineData>(Pipeline->ApiData);

    shader *ShaderStateData = (shader*) ResourceManager.Shaders.GetResource(ShaderState);
    std::shared_ptr<vkShaderData> VkShaderData = std::static_pointer_cast<vkShaderData>(ShaderStateData->ApiData);

    VkPipelineData->ShaderState = ShaderState;
    vk::DescriptorSetLayout Layouts[vkConstants::MaxDescriptorSetLayouts];

    u32 NumActiveLayouts = VkShaderData->SpirvParseResults.SetCount;
    for(sz i=0; i<NumActiveLayouts; i++)
    {
        VkPipelineData->DescriptorSetLayoutHandles[i] = CreateDescriptorSetLayout(VkShaderData->SpirvParseResults.Sets[i]);
        
        std::shared_ptr<vkResourceManagerData> VkResourceManagerData = std::static_pointer_cast<vkResourceManagerData>(ResourceManager.ApiData);
        VkPipelineData->DescriptorSetLayouts[i] = (descriptorSetLayout*)VkResourceManagerData->DescriptorSetLayouts.GetResource(VkPipelineData->DescriptorSetLayoutHandles[i]);
        Layouts[i] = VkPipelineData->DescriptorSetLayouts[i]->NativeHandle;
    }

    vk::PipelineLayoutCreateInfo PipelineLayoutCreate;
    PipelineLayoutCreate.setPSetLayouts(Layouts)
                        .setSetLayoutCount(NumActiveLayouts);

    VkPipelineData->PipelineLayout = VkData->Device.createPipelineLayout(PipelineLayoutCreate);
    VkPipelineData->NumActiveLayouts = NumActiveLayouts;

    if(ShaderStateData->GraphicsPipeline)
    {
        vk::GraphicsPipelineCreateInfo PipelineCreateInfo;

        PipelineCreateInfo.setPStages(VkShaderData->ShaderStageCreateInfo)
                          .setStageCount(ShaderStateData->ActiveShaders)
                          .setLayout(VkPipelineData->PipelineLayout);

        vk::PipelineVertexInputStateCreateInfo VertexInputInfo;
        
        vk::VertexInputAttributeDescription VertexAttributes[8];
        if(PipelineCreation.VertexInput.NumVertexAttributes)
        {
            for(u32 i=0; i<PipelineCreation.VertexInput.NumVertexAttributes; i++)
            {
                const vertexAttribute &VertexAttribute = PipelineCreation.VertexInput.VertexAttributes[i];
                VertexAttributes[i] = vk::VertexInputAttributeDescription(VertexAttribute.Location, VertexAttribute.Binding, ToVkVertexFormat(VertexAttribute.Format), VertexAttribute.Offset);
            }
            VertexInputInfo.setVertexAttributeDescriptionCount(PipelineCreation.VertexInput.NumVertexAttributes)
                           .setPVertexAttributeDescriptions(VertexAttributes);
        }
        else VertexInputInfo.setVertexAttributeDescriptionCount(0) .setPVertexAttributeDescriptions(nullptr);
        
        vk::VertexInputBindingDescription VertexBindings[8];

        if(PipelineCreation.VertexInput.NumVertexStreams)
        {
            VertexInputInfo.setVertexBindingDescriptionCount(PipelineCreation.VertexInput.NumVertexStreams);

            for(u32 i=0; i<PipelineCreation.VertexInput.NumVertexStreams; i++)
            {
                const vertexStream &VertexStream = PipelineCreation.VertexInput.VertexStreams[i];
                vk::VertexInputRate VertexRate = VertexStream.InputRate == vertexInputRate::PerVertex ? vk::VertexInputRate::eVertex : vk::VertexInputRate::eInstance;
                VertexBindings[i] = vk::VertexInputBindingDescription(VertexStream.Binding, VertexStream.Stride, VertexRate);
            }
            VertexInputInfo.setPVertexBindingDescriptions(VertexBindings);
        }
        else VertexInputInfo.setVertexBindingDescriptionCount(0).setVertexBindingDescriptions(nullptr);
        
        PipelineCreateInfo.setPVertexInputState(&VertexInputInfo);

        vk::PipelineInputAssemblyStateCreateInfo InputAssemblyCreateInfo;
        InputAssemblyCreateInfo.setTopology(vk::PrimitiveTopology::eTriangleList)
                               .setPrimitiveRestartEnable(VK_FALSE);
        PipelineCreateInfo.setPInputAssemblyState(&InputAssemblyCreateInfo);

        vk::PipelineColorBlendAttachmentState ColorBlendAttachment[8];
        if(PipelineCreation.BlendState.ActiveStates)
        {
            for(sz i=0; i<PipelineCreation.BlendState.ActiveStates; i++)
            {
                const blendState &BlendState = PipelineCreation.BlendState.BlendStates[i];

                ColorBlendAttachment[i].colorWriteMask = vk::ColorComponentFlagBits::eR |  vk::ColorComponentFlagBits::eG |  vk::ColorComponentFlagBits::eB |  vk::ColorComponentFlagBits::eA;
                ColorBlendAttachment[i].blendEnable = BlendState.BlendEnabled ? VK_TRUE : VK_FALSE;
                ColorBlendAttachment[i].srcColorBlendFactor = BlendFactorToNative(BlendState.SourceColor);
                ColorBlendAttachment[i].dstColorBlendFactor = BlendFactorToNative(BlendState.DestinationColor);
                ColorBlendAttachment[i].colorBlendOp = BlendOpToNative(BlendState.ColorOp);

                if(BlendState.SeparateBlend)
                {
                    ColorBlendAttachment[i].srcAlphaBlendFactor = BlendFactorToNative(BlendState.SourceAlpha);
                    ColorBlendAttachment[i].dstAlphaBlendFactor = BlendFactorToNative(BlendState.DestinationAlpha);
                    ColorBlendAttachment[i].alphaBlendOp = BlendOpToNative(BlendState.AlphaOp);
                }
                else{
                    ColorBlendAttachment[i].srcAlphaBlendFactor = BlendFactorToNative(BlendState.SourceColor);
                    ColorBlendAttachment[i].dstAlphaBlendFactor = BlendFactorToNative(BlendState.DestinationColor);
                    ColorBlendAttachment[i].alphaBlendOp = BlendOpToNative(BlendState.ColorOp);
                }
            }
        }
        else
        {
            for(u32 i=0; i<PipelineCreation.RenderPass.NumColorFormats; i++)
            {
                ColorBlendAttachment[i] = vk::PipelineColorBlendAttachmentState();
                ColorBlendAttachment[i].blendEnable = VK_FALSE;
                ColorBlendAttachment[i].srcColorBlendFactor = vk::BlendFactor::eOne;
                ColorBlendAttachment[i].dstColorBlendFactor = vk::BlendFactor::eZero;
                ColorBlendAttachment[i].colorWriteMask = vk::ColorComponentFlagBits::eR |  vk::ColorComponentFlagBits::eG |  vk::ColorComponentFlagBits::eB |  vk::ColorComponentFlagBits::eA;
            }
        }

        vk::PipelineColorBlendStateCreateInfo ColorBlending;
        ColorBlending.setLogicOp(vk::LogicOp::eCopy)
                     //.setAttachmentCount(PipelineCreation.BlendState.ActiveStates)
                      .setAttachmentCount(PipelineCreation.BlendState.ActiveStates ? PipelineCreation.BlendState.ActiveStates : PipelineCreation.RenderPass.NumColorFormats)
                     .setPAttachments(ColorBlendAttachment)
                     .setBlendConstants({0,0,0,0});
        PipelineCreateInfo.setPColorBlendState(&ColorBlending);


        vk::PipelineDepthStencilStateCreateInfo DepthStencil;
        DepthStencil.setDepthWriteEnable(PipelineCreation.DepthStencil.DepthWriteEnable ? true : false)
                    .setStencilTestEnable(PipelineCreation.DepthStencil.StencilEnable ? true : false)
                    .setDepthTestEnable(PipelineCreation.DepthStencil.DepthEnable ? true : false)
                    .setDepthCompareOp(CompareOpToNative(PipelineCreation.DepthStencil.DepthComparison));
        PipelineCreateInfo.setPDepthStencilState(&DepthStencil);


        vk::PipelineMultisampleStateCreateInfo MultisampleState;
        MultisampleState.setSampleShadingEnable(VK_FALSE)
                        .setRasterizationSamples(SampleCountToNative(PipelineCreation.RenderPass.SampleCount))
                        .setMinSampleShading(1.0f)
                        .setPSampleMask(nullptr)
                        .setAlphaToCoverageEnable(VK_FALSE)
                        .setAlphaToOneEnable(VK_FALSE);
        PipelineCreateInfo.setPMultisampleState(&MultisampleState);

        vk::PipelineRasterizationStateCreateInfo Rasterizer;
        Rasterizer.setDepthClampEnable(VK_FALSE)
                  .setRasterizerDiscardEnable(VK_FALSE)
                  .setPolygonMode(vk::PolygonMode::eFill)
                  .setLineWidth(1)
                  .setCullMode(CullModeToNative(PipelineCreation.Rasterization.CullMode))
                  .setFrontFace(FrontFaceToNative(PipelineCreation.Rasterization.FrontFace))
                  .setDepthBiasEnable(VK_FALSE)
                  .setDepthBiasConstantFactor(0.0f)
                  .setDepthBiasClamp(0.0f)
                  .setDepthBiasSlopeFactor(0.0f);
        PipelineCreateInfo.setPRasterizationState(&Rasterizer);

        vk::Viewport Viewport;
        Viewport.x = 0;
        Viewport.y = 0;
        Viewport.width = (float)VkData->SurfaceExtent.width;
        Viewport.height = (float)VkData->SurfaceExtent.height;
        Viewport.minDepth = 0;
        Viewport.maxDepth = 1;

        vk::Rect2D Scissor;
        Scissor.offset = vk::Offset2D(0,0);
        Scissor.extent = vk::Extent2D(VkData->SurfaceExtent.width, VkData->SurfaceExtent.height);

        vk::PipelineViewportStateCreateInfo ViewportState;
        ViewportState.setViewportCount(1)
                     .setPViewports(&Viewport)
                     .setScissorCount(1)
                     .setPScissors(&Scissor);
        PipelineCreateInfo.setPViewportState(&ViewportState);

        renderPass *RenderPass = VkData->GetRenderPass(PipelineCreation.RenderPass, std::string(PipelineCreation.RenderPass.Name));
        std::shared_ptr<vkRenderPassData> VkRenderPassData = std::static_pointer_cast<vkRenderPassData>(RenderPass->ApiData);
        PipelineCreateInfo.setRenderPass(VkRenderPassData->NativeHandle);

        vk::DynamicState DynamicStates[] = {vk::DynamicState::eViewport, vk::DynamicState::eScissor};
        vk::PipelineDynamicStateCreateInfo DynamicState;
        DynamicState.setDynamicStateCount(2)
                    .setPDynamicStates(DynamicStates);
        PipelineCreateInfo.setPDynamicState(&DynamicState);

        VkPipelineData->NativeHandle = VkData->Device.createGraphicsPipeline(VK_NULL_HANDLE, PipelineCreateInfo).value;
        VkPipelineData->BindPoint = vk::PipelineBindPoint::eGraphics;
    }
    else
    {
        vk::ComputePipelineCreateInfo ComputePipelineCreateInfo;
        ComputePipelineCreateInfo.setStage(VkShaderData->ShaderStageCreateInfo[0])
                                 .setLayout(VkPipelineData->PipelineLayout);
        VkPipelineData->NativeHandle = VkData->Device.createComputePipeline(VK_NULL_HANDLE, ComputePipelineCreateInfo).value;
        VkPipelineData->BindPoint = vk::PipelineBindPoint::eCompute;
    }

    // if(PipelineCachePath != nullptr && !CacheExists)
    // {
    //     std::vector<u8> CacheData = VkData->Device.getPipelineCacheData(PipelineCache);
    //     WriteFileBinary(PipelineCachePath, CacheData.data(), CacheData.size() * sizeof(u8));
    // }

    // VkData->Device.destroyPipelineCache(PipelineCache);
    return Handle;
}

renderPassHandle context::GetDefaultRenderPass()
{
    GET_CONTEXT(VkData, this);
    return VkData->RenderPassCache["Swapchain"];
}

framebufferHandle context::CreateFramebuffer(const framebufferCreateInfo &CreateInfo)
{
    auto Context = context::Get();
    GET_CONTEXT(VkData, Context);

    //Create the render pass
    GET_CONTEXT(VKData, this);
    renderPassOutput RenderPassOutput = {};
    RenderPassOutput.Reset();
    for (sz i = 0; i < CreateInfo.ColorFormats.size(); i++)
    {
        RenderPassOutput.Color(CreateInfo.ColorFormats[i], imageLayout::ColorAttachmentOptimal, renderPassOperation::Clear);
    }
    RenderPassOutput.Depth(CreateInfo.DepthFormat, imageLayout::DepthAttachmentOptimal);
    RenderPassOutput.SetDepthStencilOperation(renderPassOperation::Clear, renderPassOperation::Clear);
    RenderPassOutput.Name = "Foo";
    renderPass *RenderPass = VKData->GetRenderPass(RenderPassOutput, "Foo");
    RenderPass->Output = RenderPassOutput;
    vk::RenderPass VkRenderPass = std::static_pointer_cast<vkRenderPassData>(RenderPass->ApiData)->NativeHandle;

    //Create color images
    std::shared_ptr<image> *ColorImages = (std::shared_ptr<image>*)AllocateMemory(sizeof(std::shared_ptr<image>) * CreateInfo.ColorFormats.size());
    std::vector<vk::ImageView> Attachments(CreateInfo.ColorFormats.size() + 1);
    for (sz i = 0; i < CreateInfo.ColorFormats.size(); i++)
    {
        ColorImages[i] = std::make_shared<image>();
        ColorImages[i]->Init(CreateInfo.Width, CreateInfo.Height, CreateInfo.ColorFormats[i], imageUsage::COLOR_ATTACHMENT, memoryUsage::GpuOnly);
        std::shared_ptr<vkImageData> VKImage = std::static_pointer_cast<vkImageData>(ColorImages[i]->ApiData);
        Attachments[i] = VKImage->DefaultImageViews.NativeView;
    }

    //Create depth image
    std::shared_ptr<image> DepthImage = std::make_shared<image>();
    DepthImage->Init(CreateInfo.Width, CreateInfo.Height, CreateInfo.DepthFormat, imageUsage::DEPTH_STENCIL_ATTACHMENT, memoryUsage::GpuOnly);
    std::shared_ptr<vkImageData> VKDepthImage = std::static_pointer_cast<vkImageData>(DepthImage->ApiData);
    Attachments[Attachments.size()-1] = VKDepthImage->DefaultImageViews.NativeView;

    //Create the framebuffer
    vk::FramebufferCreateInfo FramebufferCreateInfo;
    FramebufferCreateInfo.setRenderPass(VkRenderPass)
                        .setAttachments(Attachments)
                        .setAttachmentCount((u32)Attachments.size())
                        .setWidth(CreateInfo.Width)
                        .setHeight(CreateInfo.Height)
                        .setLayers(1);
    framebufferHandle FramebufferHandle = Context->ResourceManager.Framebuffers.ObtainResource();
    if(FramebufferHandle == InvalidHandle)
    {
        assert(false);
        return InvalidHandle;
    }
    framebuffer *Framebuffer = (framebuffer*)Context->ResourceManager.Framebuffers.GetResource(FramebufferHandle);
    Framebuffer->Width = CreateInfo.Width;
    Framebuffer->Height = CreateInfo.Height;
    Framebuffer->ApiData = std::make_shared<vkFramebufferData>();
    Framebuffer->RenderPass = VKData->RenderPassCache["Foo"];   
    std::shared_ptr<vkFramebufferData> VkFramebufferData = std::static_pointer_cast<vkFramebufferData>(Framebuffer->ApiData);
    VkFramebufferData->DepthStencilImage = DepthImage;
    VkFramebufferData->ColorImages = ColorImages;
    VkFramebufferData->ColorImagesCount = (u32)CreateInfo.ColorFormats.size();
    VkFramebufferData->Handle = VkData->Device.createFramebuffer(FramebufferCreateInfo);

    return FramebufferHandle;;
}

framebufferHandle context::GetSwapchainFramebuffer()
{
    std::shared_ptr<vkSwapchainData> VkSwapchainData = std::static_pointer_cast<vkSwapchainData>(Swapchain->ApiData);
    framebufferHandle Framebuffer = VkSwapchainData->Framebuffers[VkSwapchainData->CurrentIndex];
    return Framebuffer;
}

vk::DescriptorSet AllocateDescriptorSet(vk::DescriptorSetLayout SetLayout, std::shared_ptr<uniformGroup> Group)
{
    GET_CONTEXT(VkData, context::Get());
    
    vk::DescriptorSetAllocateInfo AllocateInfo;
    AllocateInfo.setDescriptorPool(VkData->DescriptorPool)
                .setDescriptorSetCount(1)
                .setPSetLayouts(&SetLayout);
    
    return VkData->Device.allocateDescriptorSets(AllocateInfo).front();
}

void context::BindUniformsToPipeline(std::shared_ptr<uniformGroup> Uniforms, pipelineHandle PipelineHandle, u32 Binding){
    GET_CONTEXT(VkData, this);
    pipeline *Pipeline = (pipeline*)ResourceManager.Pipelines.GetResource(PipelineHandle);
    std::shared_ptr<vkPipelineData> VkPipeline = std::static_pointer_cast<vkPipelineData>(Pipeline->ApiData);
    std::shared_ptr<vkUniformData> VkUniformData = std::static_pointer_cast<vkUniformData>(Uniforms->ApiData);


    if(VkUniformData->DescriptorInfos.find(PipelineHandle) == VkUniformData->DescriptorInfos.end())
    {
        Uniforms->Bindings[PipelineHandle] = Binding;
        VkUniformData->DescriptorInfos[PipelineHandle] = {};
        VkUniformData->DescriptorInfos[PipelineHandle].DescriptorSetLayout = VkPipeline->DescriptorSetLayouts[Binding];
        
        //TODO: Do we really want to create a new descriptor set everytime we bind to a pipeline?
        // if(!VkUniformData->Initialized)
        {
            VkUniformData->DescriptorInfos[PipelineHandle].DescriptorSet = AllocateDescriptorSet(VkUniformData->DescriptorInfos[PipelineHandle].DescriptorSetLayout->NativeHandle, Uniforms);
            VkUniformData->Initialized=true;
        }
    }
}

void context::CopyDataToBuffer(bufferHandle BufferHandle, void *Ptr, sz Size, sz Offset)
{
    buffer *Buffer = (buffer*)ResourceManager.Buffers.GetResource(BufferHandle);
    
    Buffer->Name = "";
    std::shared_ptr<vkBufferData> VkBufferData = std::static_pointer_cast<vkBufferData>(Buffer->ApiData);
    
    auto StageBuffer = GetStageBuffer();
    auto CommandBuffer = GetImmediateCommandBuffer();

    CommandBuffer->Begin();

    auto Allocation = StageBuffer->Submit((uint8_t*)Ptr, (u32)Size);

    CommandBuffer->CopyBuffer(
        gfx::bufferInfo {StageBuffer->GetBuffer(), Allocation.Offset},
        gfx::bufferInfo {Buffer, 0},
        Allocation.Size
    );
    
    StageBuffer->Flush();
    CommandBuffer->End();

    SubmitCommandBufferImmediate(CommandBuffer);
    StageBuffer->Reset();     
}

void context::DestroyPipeline(pipelineHandle PipelineHandle)
{
    GET_CONTEXT(VkData, this);
    
    pipeline *Pipeline = (pipeline *) ResourceManager.Pipelines.GetResource(PipelineHandle);
    std::shared_ptr<vkPipelineData> VkPipelineData = std::static_pointer_cast<vkPipelineData>(Pipeline->ApiData);
    
    
    shader *Shader = (shader *) ResourceManager.Shaders.GetResource(VkPipelineData->ShaderState);
    std::shared_ptr<vkShaderData> VkShaderData = std::static_pointer_cast<vkShaderData>(Shader->ApiData);
    for (size_t i = 0; i < Shader->ActiveShaders; i++)
    {
        VkData->Device.destroyShaderModule(VkShaderData->ShaderStageCreateInfo[i].module);
    }
    ResourceManager.Shaders.ReleaseResource(VkPipelineData->ShaderState);
    
    

    VkData->Device.destroyPipeline(VkPipelineData->NativeHandle);
    VkData->Device.destroyPipelineLayout(VkPipelineData->PipelineLayout);
    ResourceManager.Pipelines.ReleaseResource(PipelineHandle);

    std::shared_ptr<vkResourceManagerData> VKResourceManager = std::static_pointer_cast<vkResourceManagerData>(ResourceManager.ApiData);
    for(sz i=0; i<VkPipelineData->NumActiveLayouts; i++)
    {
        VkData->Device.destroyDescriptorSetLayout(VkPipelineData->DescriptorSetLayouts[i]->NativeHandle);
        VKResourceManager->DescriptorSetLayouts.ReleaseResource(VkPipelineData->DescriptorSetLayoutHandles[i]);
        DeallocateMemory(VkPipelineData->DescriptorSetLayouts[i]->Bindings);
    }


}

void context::DestroyBuffer(bufferHandle BufferHandle)
{
    GET_CONTEXT(VkData, this);

    buffer *Buffer = (buffer *) ResourceManager.Buffers.GetResource(BufferHandle);
    std::shared_ptr<vkBufferData> VkBufferData = std::static_pointer_cast<vkBufferData>(Buffer->ApiData);
    vmaDestroyBuffer(VkData->Allocator, VkBufferData->Handle, VkBufferData->Allocation);
    ResourceManager.Buffers.ReleaseResource(BufferHandle);
}

void context::DestroyVertexBuffer(bufferHandle VertexBufferHandle)
{
    GET_CONTEXT(VkData, this);

    vertexBuffer *VertexBuffer = (vertexBuffer *) ResourceManager.VertexBuffers.GetResource(VertexBufferHandle);
    for (sz i = 0; i < VertexBuffer->NumVertexStreams; i++)
    {
        DestroyBuffer(VertexBuffer->VertexStreams[i].Buffer);
    }
    ResourceManager.VertexBuffers.ReleaseResource(VertexBufferHandle);
}

void context::DestroyImage(imageHandle ImageHandle)
{
    GET_CONTEXT(VkData, this);
    image *Image = (image *) ResourceManager.Images.GetResource(ImageHandle);
    std::shared_ptr<vkImageData> VkImageData = std::static_pointer_cast<vkImageData>(Image->ApiData);
    Image->Destroy();
    ResourceManager.Images.ReleaseResource(ImageHandle);
}

void context::DestroyFramebuffer(framebufferHandle FramebufferHandle)
{
    GET_CONTEXT(VkData, this);
    framebuffer *Framebuffer = (framebuffer *) ResourceManager.Framebuffers.GetResource(FramebufferHandle);
    std::shared_ptr<vkFramebufferData> VkFramebufferData = std::static_pointer_cast<vkFramebufferData>(Framebuffer->ApiData);

    for (sz i = 0; i < VkFramebufferData->ColorImagesCount; i++)
    {
        VkFramebufferData->ColorImages[i]->Destroy();
        
    }
    VkFramebufferData->DepthStencilImage->Destroy();
    
    VkData->Device.destroyFramebuffer(VkFramebufferData->Handle);
    ResourceManager.Framebuffers.ReleaseResource(FramebufferHandle);

    DeallocateMemory(VkFramebufferData->ColorImages);
}

void context::DestroySwapchain()
{
    GET_CONTEXT(VkData, this);
    
    std::shared_ptr<vkSwapchainData> VkSwapchainData = std::static_pointer_cast<vkSwapchainData>(Swapchain->ApiData);
    
    // //Destroy Framebuffers
    for (sz i = 0; i <VkSwapchainData->ImageCount; i++)
    {
        framebuffer *Framebuffer = (framebuffer*)ResourceManager.Framebuffers.GetResource(VkSwapchainData->Framebuffers[i]);
        std::shared_ptr<vkFramebufferData> VkFramebufferData = std::static_pointer_cast<vkFramebufferData>(Framebuffer->ApiData);
        VkData->Device.destroyFramebuffer(VkFramebufferData->Handle);
        ResourceManager.Framebuffers.ReleaseResource(VkSwapchainData->Framebuffers[i]);
        
        std::shared_ptr<vkImageData> VkImageData = std::static_pointer_cast<vkImageData>(VkSwapchainData->SwapchainImages[i]->ApiData);
        VkData->Device.destroyImageView(VkImageData->DefaultImageViews.NativeView);


        //Destroy the multisampled render targets
        if(VkFramebufferData->IsMultiSampled)
        {
            VkFramebufferData->MultiSampledColorImage->Destroy();
            VkFramebufferData->MultiSampledDepthStencilImage->Destroy();
        }
        if(i==0) //Only 1 depth buffer
        {
            VkFramebufferData->DepthStencilImage->Destroy();
            
        }  
    }
    VkData->Device.destroySwapchainKHR(VkSwapchainData->Handle);
}

void context::WaitIdle()
{
    GET_CONTEXT(VkData, this);
    VkData->Device.waitIdle();
}

void context::Cleanup()
{
    GET_CONTEXT(VkData, this);

    DeallocateMemory((void*)SwapchainOutput.Name);
    VkData->StageBuffer.Destroy();
    VkData->VirtualFrames.Destroy();  
    
    for(auto RenderPassHandle : VkData->RenderPassCache)
    {
        renderPass *RenderPass = (renderPass *) ResourceManager.RenderPasses.GetResource(RenderPassHandle.second);
        VkData->Device.destroyRenderPass(std::static_pointer_cast<vkRenderPassData>(RenderPass->ApiData)->NativeHandle);
        ResourceManager.RenderPasses.ReleaseResource(RenderPassHandle.second);
    }

    ResourceManager.Destroy();
    
    
    VkData->Device.freeCommandBuffers(VkData->CommandPool,  {std::static_pointer_cast<vkCommandBufferData>(VkData->ImmediateCommandBuffer->ApiData)->Handle});

    VkData->Device.destroyCommandPool(VkData->CommandPool);

    VkData->Device.destroyDescriptorPool(VkData->DescriptorPool);

    VkData->Device.destroySemaphore(VkData->ImageAvailableSemaphore);
    VkData->Device.destroySemaphore(VkData->RenderingFinishedSemaphore);
    VkData->Device.destroyFence(VkData->ImmediateFence);

    glslang::FinalizeProcess();

    vmaDestroyAllocator(VkData->Allocator);
    
    //VkData->Instance.destroyDebugUtilsMessengerEXT(VkData->DebugUtilsMessenger, nullptr, VkData->DynamicLoader);
    
    VkData->Device.destroy();
    VkData->Instance.destroySurfaceKHR(VkData->Surface);
    VkData->Instance.destroy();
}


}

#endif