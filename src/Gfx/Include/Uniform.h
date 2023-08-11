#pragma once
#include "Types.h"
#include <unordered_map>
#include <memory>

namespace gfx
{

enum uniformType
{
    UniformBuffer,
    StorageBuffer,
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
    FramebufferRenderTarget,
};

struct uniform
{
    uniformType Type;
    u32 Binding=0;
    u32 ResourceHandle;
    u32 ResourceIndex=0;
};

struct buffer;
struct image;
struct framebuffer;

struct uniformGroup
{
    static const u32 DepthAttachment = 0xffffffff;
    
    std::unordered_map<pipelineHandle, u32> Bindings; //Store where this uniform group is bound in each pipeline
    std::vector<uniform> Uniforms;
    
    uniformGroup &Reset();
    
    uniformGroup &AddUniformBuffer(u32 Binding, bufferHandle Resource);
    uniformGroup &AddStorageBuffer(u32 Binding, bufferHandle Resource);
    uniformGroup &AddTexture(u32 Binding, imageHandle Resource);
    uniformGroup &AddFramebufferRenderTarget(u32 Binding, framebufferHandle Resource, u32 TargetIndex);

    uniformGroup &Update();
    
    buffer *GetBuffer(u32 Index);
    image *GetTexture(u32 Index);
    framebuffer *GetFramebuffer(u32 Index);

    void UpdateBuffer(u32 Index, void *Data, u32 Size, u32 Offset);


    std::shared_ptr<void> ApiData;
};

}