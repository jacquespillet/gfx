#pragma once
#include "Types.h"
#include <memory>

namespace gfx
{
    
struct imageCreateInfo
{
    f32 _BorderColor[4];
    // textureFilter _MinFilter = textureFilter::LINEAR;
    // textureFilter _MaxFilter = textureFilter::LINEAR;
    // textureWrapMode _WrapS = textureWrapMode::CLAMP_TO_BORDER;
    // textureWrapMode _WrapT = textureWrapMode::CLAMP_TO_BORDER;
    // textureWrapMode _WrapR = textureWrapMode::CLAMP_TO_BORDER;
    b8 _GenerateMipmaps = false;
    // compareOp _DepthCompareOp = compareOp::LESS;

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
    void Init(u32 Width, u32 Height, format Format, imageUsage::value ImageUsage, memoryUsage MemoryUsage);
    void Init(const imageData &Image, const imageCreateInfo &CreateInfo);
        
    u32 GetMipLevelWidth(u32 MipLevel);
    u32 GetMipLevelHeight(u32 MipLevel);

//TODO: What do we do here
#if GFX_API == GFX_VK
    image(vk::Image VkImage, u32 Width, u32 Height, format Format);
#endif
    void Destroy();
};
}