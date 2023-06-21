#pragma once
#include "Types.h"

namespace gfx
{
struct image
{
    format Format;
    u32 MipLevelCount=1;
    u32 LayerCount=1;
    extent2D Extent;

    void *ApiData;
};
}