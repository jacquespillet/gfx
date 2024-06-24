#pragma once
#include "gfx/Include/Types.h"
#include <string>
#include <memory>
#include "Types.h"

namespace hlgfx
{
struct texture
{
    gfx::imageHandle Handle;
    u32 ID;
    std::string Name;
    texture() = default;
    texture(std::string Name);
    texture(std::string Name, gfx::imageHandle Handle);
    std::shared_ptr<texture> Clone();

    void Serialize(std::string FilePath);
    static std::shared_ptr<texture> Deserialize(std::string FilePath);
    ~texture();
};
}