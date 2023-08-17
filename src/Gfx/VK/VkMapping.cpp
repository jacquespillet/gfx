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
        return vk::ImageLayout::eAttachmentOptimalKHR;
    case imageUsage::FRAGNENT_SHADING_RATE_ATTACHMENT:
        return vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR;
    default:
        assert(false);
        return vk::ImageLayout::eUndefined;
    }    
}

imageUsage::bits ImageLayoutToImageUsage(imageLayout Layout)
{
    switch (Layout)
    {
    case imageLayout::Undefined:
        return imageUsage::UNKNOWN;
    case imageLayout::TransferSrcOptimal:
        return imageUsage::TRANSFER_SOURCE;
    case imageLayout::TransferDstOptimal:
        return imageUsage::TRANSFER_DESTINATION;
    case imageLayout::ShaderReadOnlyOptimal:
        return imageUsage::SHADER_READ;
    case imageLayout::General:
        return imageUsage::STORAGE;
    case imageLayout::ColorAttachmentOptimal:
        return imageUsage::COLOR_ATTACHMENT;
    case imageLayout::DepthStencilAttachmentOptimal:
        return imageUsage::DEPTH_STENCIL_ATTACHMENT;
    case imageLayout::AttachmentOptimalKHR:
        return imageUsage::INPUT_ATTACHMENT;
    case imageLayout::DepthStencilReadOnlyOptimal:
        return imageUsage::SHADER_READ;
    case imageLayout::ShadingRateOptimalNV :
        return imageUsage::FRAGNENT_SHADING_RATE_ATTACHMENT;
    default:
        assert(false);
        return imageUsage::UNKNOWN;
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
        return vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
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


vk::AccessFlags ImageLayoutToAccessFlags(imageLayout Layout)
{
    switch (Layout)
    {
        case imageLayout::Undefined:
            return vk::AccessFlagBits();
            break;
        case imageLayout::General:
            return vk::AccessFlagBits::eShaderRead;
            break;
        case imageLayout::ColorAttachmentOptimal:
            return vk::AccessFlagBits::eColorAttachmentWrite;
            break;
        case imageLayout::DepthStencilAttachmentOptimal:
            return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
            break;
        case imageLayout::DepthStencilReadOnlyOptimal:
            return vk::AccessFlagBits::eShaderRead;
            break;
        case imageLayout::ShaderReadOnlyOptimal:
            return vk::AccessFlagBits::eShaderRead;
            break;
        case imageLayout::TransferSrcOptimal:
            return vk::AccessFlagBits::eTransferRead;
            break;
        case imageLayout::TransferDstOptimal:
            return vk::AccessFlagBits::eTransferWrite;
            break;
        case imageLayout::Preinitialized:
            return vk::AccessFlagBits::eNone;
            break;
        case imageLayout::DepthReadOnlyStencilAttachmentOptimal:
            return vk::AccessFlagBits::eDepthStencilAttachmentRead;
            break;
        case imageLayout::DepthAttachmentStencilReadOnlyOptimal:
            return vk::AccessFlagBits::eDepthStencilAttachmentRead;
            break;
        case imageLayout::DepthAttachmentOptimal:
            return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
            break;
        case imageLayout::DepthReadOnlyOptimal:
            return vk::AccessFlagBits::eDepthStencilAttachmentRead;
            break;
        case imageLayout::StencilAttachmentOptimal:
            return vk::AccessFlagBits::eDepthStencilAttachmentWrite;
            break;
        case imageLayout::StencilReadOnlyOptimal:
            return vk::AccessFlagBits::eDepthStencilAttachmentRead;
            break;
        case imageLayout::ReadOnlyOptimal:
            return vk::AccessFlagBits::eShaderRead;
            break;
        case imageLayout::AttachmentOptimal:
            return vk::AccessFlagBits::eColorAttachmentWrite;
            break;
        case imageLayout::PresentSrcKHR:
        case imageLayout::VideoDecodeDstKHR:
        case imageLayout::VideoDecodeSrcKHR:
        case imageLayout::VideoDecodeDpbKHR:
        case imageLayout::SharedPresentKHR:
        case imageLayout::ShadingRateOptimalNV:
        case imageLayout::FragmentDensityMapOptimalEXT:
        case imageLayout::DepthAttachmentOptimalKHR:
        case imageLayout::DepthReadOnlyOptimalKHR:
        case imageLayout::StencilAttachmentOptimalKHR:
        case imageLayout::StencilReadOnlyOptimalKHR:
        case imageLayout::ReadOnlyOptimalKHR:
        case imageLayout::AttachmentOptimalKHR:
            return vk::AccessFlagBits::eNone;
            break;
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
        return vk::PipelineStageFlagBits::eFragmentShader; 
    case imageUsage::STORAGE:
        return vk::PipelineStageFlagBits::eFragmentShader; 
    case imageUsage::COLOR_ATTACHMENT:
        return vk::PipelineStageFlagBits::eColorAttachmentOutput;
    case imageUsage::DEPTH_STENCIL_ATTACHMENT:
        return vk::PipelineStageFlagBits::eEarlyFragmentTests; 
    case imageUsage::INPUT_ATTACHMENT:
        return vk::PipelineStageFlagBits::eFragmentShader; 
    case imageUsage::FRAGNENT_SHADING_RATE_ATTACHMENT:
        return vk::PipelineStageFlagBits::eFragmentShadingRateAttachmentKHR;
    default:
        assert(false);
        return vk::PipelineStageFlags{ };
    }    
}
vk::PipelineStageFlags ImageLayoutToPipelineStage(imageLayout Layout)
{
    imageUsage::bits Usage = ImageLayoutToImageUsage(Layout);
    return ImageUsageToPipelineStage(Usage);
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


static vk::Filter SamplerFilterTable[] = 
{
    vk::Filter::eNearest,//Nearest,
    vk::Filter::eLinear,//Linear,
    vk::Filter::eNearest,//NearestMipmapNearest,
    vk::Filter::eLinear,//LinearMipmapNearest,
    vk::Filter::eNearest,//NearestMipmapLinear,
    vk::Filter::eLinear,//LinearMipmapLinear
};

vk::Filter SamplerFilterToNative(samplerFilter Filter)
{
    return SamplerFilterTable[(sz)Filter];
}


static vk::SamplerMipmapMode SamplerFilterTableMip[] = 
{
    vk::SamplerMipmapMode::eNearest,//Nearest,
    vk::SamplerMipmapMode::eLinear,//Linear,
    vk::SamplerMipmapMode::eNearest,//NearestMipmapNearest,
    vk::SamplerMipmapMode::eLinear,//LinearMipmapNearest,
    vk::SamplerMipmapMode::eLinear,//NearestMipmapLinear,
    vk::SamplerMipmapMode::eLinear,//LinearMipmapLinear
};
vk::SamplerMipmapMode SamplerFilterToNativeMip(samplerFilter MinFilter)
{
    return SamplerFilterTableMip[(sz)MinFilter];
}


static vk::SamplerAddressMode SamplerWrapModeTable[] = 
{
    vk::SamplerAddressMode::eClampToEdge,//ClampToEdge,
    vk::SamplerAddressMode::eClampToBorder,//ClampToBorder,
    vk::SamplerAddressMode::eMirroredRepeat,//MirroredRepeat,
    vk::SamplerAddressMode::eRepeat,//Repeat,
    vk::SamplerAddressMode::eMirrorClampToEdge,//MirrorClampToEdge
};

vk::SamplerAddressMode SamplerWrapModeToNative(samplerWrapMode Mode)
{
    return SamplerWrapModeTable[(sz)Mode];
}


vk::IndexType IndexTypeToNative(indexType Type)
{
    return (Type == indexType::Uint16) ? vk::IndexType::eUint16 : vk::IndexType::eUint32;
}

vk::SampleCountFlagBits SampleCountToNative(u32 SampleCount)
{
    if(SampleCount == 1) return vk::SampleCountFlagBits::e1;
    else return vk::SampleCountFlagBits::e4;
}

}

#endif