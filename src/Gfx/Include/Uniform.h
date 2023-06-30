#pragma once
#include "Types.h"

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
    u32 Binding = 0;
    void Initialize();
    void Update();
    std::shared_ptr<void> ApiData;
};

}