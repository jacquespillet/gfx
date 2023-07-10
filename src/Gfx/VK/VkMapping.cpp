#if GFX_API == GFX_VK
#include "VkMapping.h"

namespace gfx
{
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


vk::Format &FormatToNative(format Format)
{
    return FormatTable[(u64)Format];
}



vk::ImageAspectFlags ImageFormatToImageAspect(format Format)
{
    switch (Format)
    {
    case format::D16_UNORM:
        return vk::ImageAspectFlagBits::eDepth;
    case format::X8D24_UNORM_PACK_32:
        return vk::ImageAspectFlagBits::eDepth;
    case format::D32_SFLOAT:
        return vk::ImageAspectFlagBits::eDepth;
    case format::D16_UNORM_S8_UINT:
        return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
    case format::D24_UNORM_S8_UINT:
        return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
    case format::D32_SFLOAT_S8_UINT:
        return vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil;
    default:
        return vk::ImageAspectFlagBits::eColor;
    }
}



vk::ImageLayout ImageUsageToImageLayout(imageUsage::bits Usage)
{
    switch (Usage)
    {
    case imageUsage::UNKNOWN:
        return vk::ImageLayout::eUndefined;
    case imageUsage::TRANSFER_SOURCE:
        return vk::ImageLayout::eTransferSrcOptimal;
    case imageUsage::TRANSFER_DESTINATION:
        return vk::ImageLayout::eTransferDstOptimal;
    case imageUsage::SHADER_READ:
        return vk::ImageLayout::eShaderReadOnlyOptimal;
    case imageUsage::STORAGE:
        return vk::ImageLayout::eGeneral;
    case imageUsage::COLOR_ATTACHMENT:
        return vk::ImageLayout::eColorAttachmentOptimal;
    case imageUsage::DEPTH_STENCIL_ATTACHMENT:
        return vk::ImageLayout::eDepthStencilAttachmentOptimal;
    case imageUsage::INPUT_ATTACHMENT:
        return vk::ImageLayout::eAttachmentOptimalKHR; // TODO: is it ok?
    case imageUsage::FRAGNENT_SHADING_RATE_ATTACHMENT:
        return vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR;
    default:
        assert(false);
        return vk::ImageLayout::eUndefined;
    }    
}



vk::AccessFlags ImageUsageToAccessFlags(imageUsage::bits Usage)
{
    switch (Usage)
    {
    case imageUsage::UNKNOWN:
        return vk::AccessFlags{ };
    case imageUsage::TRANSFER_SOURCE:
        return vk::AccessFlagBits::eTransferRead;
    case imageUsage::TRANSFER_DESTINATION:
        return vk::AccessFlagBits::eTransferWrite;
    case imageUsage::SHADER_READ:
        return vk::AccessFlagBits::eShaderRead;
    case imageUsage::STORAGE:
        return vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite; // TODO: what if storage is not read or write?
    case imageUsage::COLOR_ATTACHMENT:
        return vk::AccessFlagBits::eColorAttachmentWrite;
    case imageUsage::DEPTH_STENCIL_ATTACHMENT:
        return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    case imageUsage::INPUT_ATTACHMENT:
        return vk::AccessFlagBits::eInputAttachmentRead;
    case imageUsage::FRAGNENT_SHADING_RATE_ATTACHMENT:
        return vk::AccessFlagBits::eFragmentShadingRateAttachmentReadKHR;
    default:
        assert(false);
        return vk::AccessFlags{ };
    }    
}


vk::PipelineStageFlags ImageUsageToPipelineStage(imageUsage::bits Usage)
{
    switch (Usage)
    {
    case imageUsage::UNKNOWN:
        return vk::PipelineStageFlagBits::eTopOfPipe;
    case imageUsage::TRANSFER_SOURCE:
        return vk::PipelineStageFlagBits::eTransfer;
    case imageUsage::TRANSFER_DESTINATION:
        return vk::PipelineStageFlagBits::eTransfer;
    case imageUsage::SHADER_READ:
        return vk::PipelineStageFlagBits::eFragmentShader; // TODO: whats for vertex shader reads?
    case imageUsage::STORAGE:
        return vk::PipelineStageFlagBits::eFragmentShader; // TODO: whats for vertex shader reads?
    case imageUsage::COLOR_ATTACHMENT:
        return vk::PipelineStageFlagBits::eColorAttachmentOutput;
    case imageUsage::DEPTH_STENCIL_ATTACHMENT:
        return vk::PipelineStageFlagBits::eEarlyFragmentTests; // TODO: whats for late fragment test?
    case imageUsage::INPUT_ATTACHMENT:
        return vk::PipelineStageFlagBits::eFragmentShader; // TODO: check if at least works
    case imageUsage::FRAGNENT_SHADING_RATE_ATTACHMENT:
        return vk::PipelineStageFlagBits::eFragmentShadingRateAttachmentKHR;
    default:
        assert(false);
        return vk::PipelineStageFlags{ };
    }    
}


vk::ShaderStageFlagBits ShaderStageToNative(shaderStageFlags::bits Stage)
{
    return (vk::ShaderStageFlagBits)Stage;
}

vk::BlendFactor BlendFactorToNative(blendFactor Factor)
{
    return BlendFactorTable[(uint32_t)Factor];
}

vk::BlendOp BlendOpToNative(blendOperation Op)
{
    return BlendOpTable[(uint32_t)Op];
}

vk::CompareOp CompareOpToNative(compareOperation Op)
{
    return CompareOpTable[(uint32_t)Op];
}

vk::CullModeFlags CullModeToNative(cullMode::bits CullMode)
{
    return (vk::CullModeFlags)CullMode;
}


vk::FrontFace FrontFaceToNative(frontFace Face )
{
    return FrontFaceTable[(uint32_t)Face];
}


vk::ImageLayout ImageLayoutToNative(imageLayout ImageLayout )
{
    return ImageLayoutTable[(uint32_t)ImageLayout];
}

imageLayout ImageLayoutFromNative(const vk::ImageLayout &VkImageLayout)
{
    for(u64 i=0; i<std::size(ImageLayoutTable); i++)
    {
        if(ImageLayoutTable[i] == VkImageLayout)
        {
            return (imageLayout)i;
        }
    }
    assert(false);
    return (imageLayout)0;    
}


}

#endif