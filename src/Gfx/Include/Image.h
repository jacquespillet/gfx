#pragma once
#include "Types.h"
#include <memory>

namespace gfx
{
struct image
{
    format Format;
    u32 MipLevelCount=1;
    u32 LayerCount=1;
    extent2D Extent;

    std::shared_ptr<void> ApiData;
};
}