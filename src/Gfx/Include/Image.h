#pragma once
#include "Types.h"
#include <memory>

namespace gfx
{

//

 

struct imageCreateInfo
{
    f32 BorderColor[4];
    samplerFilter MinFilter = samplerFilter::Linear;   
    samplerFilter MagFilter = samplerFilter::Linear;
    samplerWrapMode WrapS = samplerWrapMode::ClampToBorder;
    samplerWrapMode WrapT = samplerWrapMode::ClampToBorder;
    samplerWrapMode WrapR = samplerWrapMode::ClampToBorder;
    b8 GenerateMipmaps = false;
    compareOperation DepthCompareOp = compareOperation::Less;

    u8 MipLevelCount=1;
    // static textureCreateInfo Default();
};

struct imageData
{
    format Format;
    type Type;
    u8 *Data;
    u32 Width;
    u32 Height;
    s32 ChannelCount;
    size_t DataSize;
};

imageData ImageFromFile(char *FileName);

struct image
{
    format Format;
    u32 MipLevelCount=1;
    u32 LayerCount=1;
    extent2D Extent;
    sz ByteSize=0;

    std::shared_ptr<void> ApiData;
    image() = default;
    void Init(u32 Width, u32 Height, format Format, imageUsage::value ImageUsage, memoryUsage MemoryUsage, u32 SampleCount=1);
    void Init(const imageData &Image, const imageCreateInfo &CreateInfo);
    void InitAsCubemap(const imageData &Left, const imageData &Right, const imageData &Top, const imageData &Bottom, const imageData &Back, const imageData &Front, const imageCreateInfo &CreateInfo);
        
    u32 GetMipLevelWidth(u32 MipLevel);
    u32 GetMipLevelHeight(u32 MipLevel);

//TODO: What do we do here
#if GFX_API == GFX_VK
    image(vk::Image VkImage, u32 Width, u32 Height, format Format);
#endif
    void Destroy();
};
}