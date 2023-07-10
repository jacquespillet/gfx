#pragma once
#include "Types.h"
#include <unordered_map>
#include <memory>

namespace gfx
{

enum uniformType
{
    Buffer,
    Texture2d,
    Float,
    Int,
    Vec2f,
    Vec3f,
    Vec4f,
    Vec2i,
    Vec3i,
    Vec4i,
    Vec2ui,
    Vec3ui,
    Vec4ui,
};

struct uniform
{
    char *Name;
    uniformType Type;
    u32 Binding=0;
    std::shared_ptr<void> Resource;
};

struct uniformGroup
{
    std::vector<uniform> Uniforms;
    std::unordered_map<pipelineHandle, u32> Bindings; //Store where this uniform group is bound in each pipeline
    void Initialize();
    void Update();
    std::shared_ptr<void> ApiData;
};

}