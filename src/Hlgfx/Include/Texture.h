#pragma once
#include <gfx/Include/Types.h>
#include <string>

namespace hlgfx
{
struct texture
{
    gfx::imageHandle Handle;
    std::string UUID;
    std::string Name;
    texture(std::string Name);
    texture(std::string Name, gfx::imageHandle Handle);
    ~texture();
};
}