#pragma once

#include <vulkan/vulkan.hpp>
#include "../Include/Types.h"

namespace gfx
{
//
constexpr vk::PhysicalDeviceType DeviceTypeMapping[] = 
{
    vk::PhysicalDeviceType::eCpu,
    vk::PhysicalDeviceType::eDiscreteGpu,
    vk::PhysicalDeviceType::eIntegratedGpu,
    vk::PhysicalDeviceType::eVirtualGpu,
    vk::PhysicalDeviceType::eOther,
};


struct imageUsage
{
    using value = u32;

    enum bits : value
    {
        UNKNOWN = (value) vk::ImageUsageFlagBits(),
        TRANSFER_SOURCE = (value)vk::ImageUsageFlagBits::eTransferSrc,
        TRANSFER_DESTINATION = (value)vk::ImageUsageFlagBits::eTransferDst,
        SHADER_READ = (value)vk::ImageUsageFlagBits::eSampled,
        STORAGE = (value)vk::ImageUsageFlagBits::eStorage,
        COLOR_ATTACHMENT = (value)vk::ImageUsageFlagBits::eColorAttachment,
        DEPTH_STENCIL_ATTACHMENT = (value)vk::ImageUsageFlagBits::eDepthStencilAttachment,
        INPUT_ATTACHMENT = (value)vk::ImageUsageFlagBits::eInputAttachment,
        FRAGNENT_SHADING_RATE_ATTACHMENT = (value)vk::ImageUsageFlagBits::eFragmentShadingRateAttachmentKHR,
    };
};


static vk::Format FormatTable[] = 
{
    vk::Format::eUndefined,
    vk::Format::eR4G4UnormPack8,
    vk::Format::eR4G4B4A4UnormPack16,
    vk::Format::eB4G4R4A4UnormPack16,
    vk::Format::eR5G6B5UnormPack16,
    vk::Format::eB5G6R5UnormPack16,
    vk::Format::eR5G5B5A1UnormPack16,
    vk::Format::eB5G5R5A1UnormPack16,
    vk::Format::eA1R5G5B5UnormPack16,
    vk::Format::eR8Unorm,
    vk::Format::eR8Snorm,
    vk::Format::eR8Uscaled,
    vk::Format::eR8Sscaled,
    vk::Format::eR8Uint,
    vk::Format::eR8Sint,
    vk::Format::eR8Srgb,
    vk::Format::eR8G8Unorm,
    vk::Format::eR8G8Snorm,
    vk::Format::eR8G8Uscaled,
    vk::Format::eR8G8Sscaled,
    vk::Format::eR8G8Uint,
    vk::Format::eR8G8Sint,
    vk::Format::eR8G8Srgb,
    vk::Format::eR8G8B8Unorm,
    vk::Format::eR8G8B8Snorm,
    vk::Format::eR8G8B8Uscaled,
    vk::Format::eR8G8B8Sscaled,
    vk::Format::eR8G8B8Uint,
    vk::Format::eR8G8B8Sint,
    vk::Format::eR8G8B8Srgb,
    vk::Format::eB8G8R8Unorm,
    vk::Format::eB8G8R8Snorm,
    vk::Format::eB8G8R8Uscaled,
    vk::Format::eB8G8R8Sscaled,
    vk::Format::eB8G8R8Uint,
    vk::Format::eB8G8R8Sint,
    vk::Format::eB8G8R8Srgb,
    vk::Format::eR8G8B8A8Unorm,
    vk::Format::eR8G8B8A8Snorm,
    vk::Format::eR8G8B8A8Uscaled,
    vk::Format::eR8G8B8A8Sscaled,
    vk::Format::eR8G8B8A8Uint,
    vk::Format::eR8G8B8A8Sint,
    vk::Format::eR8G8B8A8Srgb,
    vk::Format::eB8G8R8A8Unorm,
    vk::Format::eB8G8R8A8Snorm,
    vk::Format::eB8G8R8A8Uscaled,
    vk::Format::eB8G8R8A8Sscaled,
    vk::Format::eB8G8R8A8Uint,
    vk::Format::eB8G8R8A8Sint,
    vk::Format::eB8G8R8A8Srgb,
    vk::Format::eA8B8G8R8UnormPack32,
    vk::Format::eA8B8G8R8SnormPack32,
    vk::Format::eA8B8G8R8UscaledPack32,
    vk::Format::eA8B8G8R8SscaledPack32,
    vk::Format::eA8B8G8R8UintPack32,
    vk::Format::eA8B8G8R8SintPack32,
    vk::Format::eA8B8G8R8SrgbPack32,
    vk::Format::eA2R10G10B10UnormPack32,
    vk::Format::eA2R10G10B10SnormPack32,
    vk::Format::eA2R10G10B10UscaledPack32,
    vk::Format::eA2R10G10B10SscaledPack32,
    vk::Format::eA2R10G10B10UintPack32,
    vk::Format::eA2R10G10B10SintPack32,
    vk::Format::eA2B10G10R10UnormPack32,
    vk::Format::eA2B10G10R10SnormPack32,
    vk::Format::eA2B10G10R10UscaledPack32,
    vk::Format::eA2B10G10R10SscaledPack32,
    vk::Format::eA2B10G10R10UintPack32,
    vk::Format::eA2B10G10R10SintPack32,
    vk::Format::eR16Unorm,
    vk::Format::eR16Snorm,
    vk::Format::eR16Uscaled,
    vk::Format::eR16Sscaled,
    vk::Format::eR16Uint,
    vk::Format::eR16Sint,
    vk::Format::eR16Sfloat,
    vk::Format::eR16G16Unorm,
    vk::Format::eR16G16Snorm,
    vk::Format::eR16G16Uscaled,
    vk::Format::eR16G16Sscaled,
    vk::Format::eR16G16Uint,
    vk::Format::eR16G16Sint,
    vk::Format::eR16G16Sfloat,
    vk::Format::eR16G16B16Unorm,
    vk::Format::eR16G16B16Snorm,
    vk::Format::eR16G16B16Uscaled,
    vk::Format::eR16G16B16Sscaled,
    vk::Format::eR16G16B16Uint,
    vk::Format::eR16G16B16Sint,
    vk::Format::eR16G16B16Sfloat,
    vk::Format::eR16G16B16A16Unorm,
    vk::Format::eR16G16B16A16Snorm,
    vk::Format::eR16G16B16A16Uscaled,
    vk::Format::eR16G16B16A16Sscaled,
    vk::Format::eR16G16B16A16Uint,
    vk::Format::eR16G16B16A16Sint,
    vk::Format::eR16G16B16A16Sfloat,
    vk::Format::eR32Uint,
    vk::Format::eR32Sint,
    vk::Format::eR32Sfloat,
    vk::Format::eR32G32Uint,
    vk::Format::eR32G32Sint,
    vk::Format::eR32G32Sfloat,
    vk::Format::eR32G32B32Uint,
    vk::Format::eR32G32B32Sint,
    vk::Format::eR32G32B32Sfloat,
    vk::Format::eR32G32B32A32Uint,
    vk::Format::eR32G32B32A32Sint,
    vk::Format::eR32G32B32A32Sfloat,
    vk::Format::eR64Uint,
    vk::Format::eR64Sint,
    vk::Format::eR64Sfloat,
    vk::Format::eR64G64Uint,
    vk::Format::eR64G64Sint,
    vk::Format::eR64G64Sfloat,
    vk::Format::eR64G64B64Uint,
    vk::Format::eR64G64B64Sint,
    vk::Format::eR64G64B64Sfloat,
    vk::Format::eR64G64B64A64Uint,
    vk::Format::eR64G64B64A64Sint,
    vk::Format::eR64G64B64A64Sfloat,
    vk::Format::eB10G11R11UfloatPack32,
    vk::Format::eE5B9G9R9UfloatPack32,
    vk::Format::eD16Unorm,
    vk::Format::eX8D24UnormPack32,
    vk::Format::eD32Sfloat,
    vk::Format::eS8Uint,
    vk::Format::eD16UnormS8Uint,
    vk::Format::eD24UnormS8Uint,
    vk::Format::eD32SfloatS8Uint,        
};

format FormatFromNative(const vk::Format &VkFormat)
{
    for(u64 i=0; i<std::size(FormatTable); i++)
    {
        if(FormatTable[i] == VkFormat)
        {
            return (format)i;
        }
    }
    assert(false);
    return (format)0;    
}

}

