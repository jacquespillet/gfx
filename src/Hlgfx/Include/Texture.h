#pragma once
#include <gfx/Include/Types.h>

namespace hlgfx
{
struct texture
{
    gfx::imageHandle Handle;
    texture();
    texture(gfx::imageHandle Handle);
    ~texture();
};
}