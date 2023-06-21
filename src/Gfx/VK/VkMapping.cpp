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

}

#endif