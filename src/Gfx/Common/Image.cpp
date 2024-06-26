#include "../Include/Image.h"
#include "../Include/Memory.h"
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace gfx

{
imageData ImageFromFile(char *FileName)
{
    imageData Result;

    s32 Width, Height, ChannelCount;
    
    u8 *Data = stbi_load(FileName, &Width, &Height, &ChannelCount, 4);
    Result.Data = (u8*)AllocateMemory(Width * Height * 4);
    memcpy(Result.Data, Data, Width * Height * 4);
    assert(Result.Data);
    Result.Width = Width;
    Result.Height = Height;
    Result.ChannelCount = 4;
    Result.Format = format::R8G8B8A8_UNORM;
    Result.Type = type::UNSIGNED_BYTE;
    Result.DataSize = Width * Height * Result.ChannelCount * sizeof(u8);
    stbi_image_free(Data);
    return Result;
}
u32 image::GetMipLevelWidth(u32 MipLevel)
{
    u32 SafeWidth = (std::max)(Extent.Width, 1u << MipLevel);
    return SafeWidth >> MipLevel;
}

u32 image::GetMipLevelHeight(u32 MipLevel)
{
    u32 SafeHeight = (std::max)(Extent.Height, 1u << MipLevel);
    return SafeHeight >> MipLevel;
}

}
