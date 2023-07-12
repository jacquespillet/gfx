#pragma once

#include <vulkan/vulkan.hpp>
#include "../Include/Types.h"
#include "../Include/Pipeline.h"

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

format FormatFromNative(const vk::Format &VkFormat);
vk::Format &FormatToNative(format Format);

vk::ShaderStageFlagBits ShaderStageToNative(shaderStageFlags::bits Stage);

vk::ImageAspectFlags ImageFormatToImageAspect(format Format);



vk::ImageLayout ImageUsageToImageLayout(imageUsage::bits Usage);
vk::AccessFlags ImageUsageToAccessFlags(imageUsage::bits Usage);
vk::PipelineStageFlags ImageUsageToPipelineStage(imageUsage::bits Usage);

static vk::Format ToVkVertexFormat( vertexComponentFormat::values Value ) {
    static vk::Format Mapping[ vertexComponentFormat::Count ] = { vk::Format::eR32Sfloat, vk::Format::eR32G32Sfloat, vk::Format::eR32G32B32Sfloat, vk::Format::eR32G32B32A32Sfloat, /*MAT4 TODO*/vk::Format::eR32G32B32A32Sfloat,
                                                                          vk::Format::eR8Sint, vk::Format::eR8G8B8A8Snorm, vk::Format::eR8Uint, vk::Format::eR8G8B8A8Uint, vk::Format::eR16G16Sint, vk::Format::eR16G16Snorm,
                                                                          vk::Format::eR16G16B16A16Sint, vk::Format::eR16G16B16A16Snorm, vk::Format::eR32Uint, vk::Format::eR32G32Uint, vk::Format::eR32G32B32A32Uint };
    return Mapping[ Value ];
}



static vk::BlendFactor BlendFactorTable[] = 
{
    vk::BlendFactor::eZero,
    vk::BlendFactor::eOne,
    vk::BlendFactor::eSrcColor,
    vk::BlendFactor::eOneMinusSrcColor,
    vk::BlendFactor::eDstColor,
    vk::BlendFactor::eOneMinusDstColor,
    vk::BlendFactor::eSrcAlpha,
    vk::BlendFactor::eOneMinusSrcAlpha,
    vk::BlendFactor::eDstAlpha,
    vk::BlendFactor::eOneMinusDstAlpha,
    vk::BlendFactor::eConstantColor,
    vk::BlendFactor::eOneMinusConstantColor,
    vk::BlendFactor::eConstantAlpha,
    vk::BlendFactor::eOneMinusConstantAlpha,
    vk::BlendFactor::eSrcAlphaSaturate,
    vk::BlendFactor::eSrc1Color,
    vk::BlendFactor::eOneMinusSrc1Color,
    vk::BlendFactor::eSrc1Alpha,
    vk::BlendFactor::eOneMinusSrc1Alpha
};

vk::BlendFactor BlendFactorToNative(blendFactor Factor);

static vk::BlendOp BlendOpTable[] = 
{
    vk::BlendOp::eAdd,
    vk::BlendOp::eSubtract,
    vk::BlendOp::eReverseSubtract,
    vk::BlendOp::eMin,
    vk::BlendOp::eMax,
    vk::BlendOp::eZeroEXT,
    vk::BlendOp::eSrcEXT,
    vk::BlendOp::eDstEXT,
    vk::BlendOp::eSrcOverEXT,
    vk::BlendOp::eDstOverEXT,
    vk::BlendOp::eSrcInEXT,
    vk::BlendOp::eDstInEXT,
    vk::BlendOp::eSrcOutEXT,
    vk::BlendOp::eDstOutEXT,
    vk::BlendOp::eSrcAtopEXT,
    vk::BlendOp::eDstAtopEXT,
    vk::BlendOp::eXorEXT,
    vk::BlendOp::eMultiplyEXT,
    vk::BlendOp::eScreenEXT,
    vk::BlendOp::eOverlayEXT,
    vk::BlendOp::eDarkenEXT,
    vk::BlendOp::eLightenEXT,
    vk::BlendOp::eColordodgeEXT,
    vk::BlendOp::eColorburnEXT,
    vk::BlendOp::eHardlightEXT,
    vk::BlendOp::eSoftlightEXT,
    vk::BlendOp::eDifferenceEXT,
    vk::BlendOp::eExclusionEXT,
    vk::BlendOp::eInvertEXT,
    vk::BlendOp::eInvertRgbEXT,
    vk::BlendOp::eLineardodgeEXT,
    vk::BlendOp::eLinearburnEXT,
    vk::BlendOp::eVividlightEXT,
    vk::BlendOp::eLinearlightEXT,
    vk::BlendOp::ePinlightEXT,
    vk::BlendOp::eHardmixEXT,
    vk::BlendOp::eHslHueEXT,
    vk::BlendOp::eHslSaturationEXT,
    vk::BlendOp::eHslColorEXT,
    vk::BlendOp::eHslLuminosityEXT,
    vk::BlendOp::ePlusEXT,
    vk::BlendOp::ePlusClampedEXT,
    vk::BlendOp::ePlusClampedAlphaEXT,
    vk::BlendOp::ePlusDarkerEXT,
    vk::BlendOp::eMinusEXT,
    vk::BlendOp::eMinusClampedEXT,
    vk::BlendOp::eContrastEXT,
    vk::BlendOp::eInvertOvgEXT,
    vk::BlendOp::eRedEXT,
    vk::BlendOp::eGreenEXT,
    vk::BlendOp::eBlueEXT
};

vk::BlendOp BlendOpToNative(blendOperation Op);

static vk::CompareOp CompareOpTable[] = 
{    
    vk::CompareOp::eNever,
    vk::CompareOp::eLess,
    vk::CompareOp::eEqual,
    vk::CompareOp::eLessOrEqual,
    vk::CompareOp::eGreater,
    vk::CompareOp::eNotEqual,
    vk::CompareOp::eGreaterOrEqual,
    vk::CompareOp::eAlways
};

vk::CompareOp CompareOpToNative(compareOperation Op);

vk::CullModeFlags CullModeToNative(cullMode::bits CullMode);

static vk::FrontFace FrontFaceTable[] = 
{    
    vk::FrontFace::eCounterClockwise,
    vk::FrontFace::eClockwise
};

vk::FrontFace FrontFaceToNative(frontFace Face );


static vk::ImageLayout ImageLayoutTable[] = 
{
    vk::ImageLayout::eUndefined,
    vk::ImageLayout::eGeneral,
    vk::ImageLayout::eColorAttachmentOptimal,
    vk::ImageLayout::eDepthStencilAttachmentOptimal,
    vk::ImageLayout::eDepthStencilReadOnlyOptimal,
    vk::ImageLayout::eShaderReadOnlyOptimal,
    vk::ImageLayout::eTransferSrcOptimal,
    vk::ImageLayout::eTransferDstOptimal,
    vk::ImageLayout::ePreinitialized,
    vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal,
    vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal,
    vk::ImageLayout::eDepthAttachmentOptimal,
    vk::ImageLayout::eDepthReadOnlyOptimal,
    vk::ImageLayout::eStencilAttachmentOptimal,
    vk::ImageLayout::eStencilReadOnlyOptimal,
    vk::ImageLayout::eReadOnlyOptimal,
    vk::ImageLayout::eAttachmentOptimal,
    vk::ImageLayout::ePresentSrcKHR,
    vk::ImageLayout::eVideoDecodeDstKHR,
    vk::ImageLayout::eVideoDecodeSrcKHR,
    vk::ImageLayout::eVideoDecodeDpbKHR,
    vk::ImageLayout::eSharedPresentKHR,
    vk::ImageLayout::eShadingRateOptimalNV,
    vk::ImageLayout::eFragmentDensityMapOptimalEXT,
    vk::ImageLayout::eDepthAttachmentOptimalKHR,
    vk::ImageLayout::eDepthReadOnlyOptimalKHR,
    vk::ImageLayout::eStencilAttachmentOptimalKHR,
    vk::ImageLayout::eStencilReadOnlyOptimalKHR,
    vk::ImageLayout::eReadOnlyOptimalKHR,
    vk::ImageLayout::eAttachmentOptimalKHR    
};


vk::ImageLayout ImageLayoutToNative(imageLayout ImageLayout );

imageLayout ImageLayoutFromNative(const vk::ImageLayout &VkImageLayout);

vk::Filter SamplerFilterToNative(samplerFilter Filter);
vk::SamplerMipmapMode SamplerFilterToNativeMip(samplerFilter MinFilter);

vk::SamplerAddressMode SamplerWrapModeToNative(samplerWrapMode Mode);
}

