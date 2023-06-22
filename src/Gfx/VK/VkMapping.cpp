#if API == VK
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