#pragma once
#include <iostream>
#include <optional>

#include "../../App/App.h"
#include "../Include/GfxContext.h"
#include "VkGfxContext.h"
namespace gfx
{
    
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

}