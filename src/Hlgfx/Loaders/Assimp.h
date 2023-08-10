#pragma once
#include <vector>
#include <memory>
#include <string>

namespace hlgfx
{
struct object3D;

namespace loaders
{
namespace assimp
{
    std::shared_ptr<object3D> Load(std::string FileName);
}
}
}