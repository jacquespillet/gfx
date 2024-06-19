#include "../Include/Uniform.h"
#include "../Include/Buffer.h"
#include "../Include/Image.h"
#include "../Include/Framebuffer.h"
#include "../Include/Context.h"

namespace gfx
{

uniformGroup &uniformGroup::AddUniformBuffer(u32 Binding, bufferHandle Resource)
{
    this->Uniforms.push_back({
        uniformType::UniformBuffer,
        Binding,
        Resource,
        0
    });
    return *this;
}

uniformGroup &uniformGroup::AddStorageBuffer(u32 Binding, bufferHandle Resource)
{
    this->Uniforms.push_back({
        uniformType::StorageBuffer,
        Binding,
        Resource,
        0
    });
    return *this;
}

uniformGroup &uniformGroup::AddTexture(u32 Binding, imageHandle Resource)
{
    this->Uniforms.push_back({
        uniformType::Texture2d,
        Binding,
        Resource,
        0
    });
    return *this;
}

uniformGroup &uniformGroup::AddStorageImage(u32 Binding, imageHandle Resource)
{
    this->Uniforms.push_back({
        uniformType::StorageImage,
        Binding,
        Resource,
        0
    });
    return *this;
}


#if GFX_API==GFX_VK || GFX_API==GFX_D3D12
uniformGroup &uniformGroup::AddAccelerationStructure(u32 Binding, accelerationStructureHandle Resource)
{
    this->Uniforms.push_back({
        uniformType::AccelerationStructure,
        Binding,
        Resource,
        0
    });
    return *this;
}
#endif

uniformGroup &uniformGroup::AddFramebufferRenderTarget(u32 Binding, framebufferHandle Resource, u32 ResourceIndex)
{
    this->Uniforms.push_back({
        uniformType::FramebufferRenderTarget,
        Binding,
        Resource,
        ResourceIndex
    });
    return *this;
}


buffer *uniformGroup::GetBuffer(u32 Index)
{
    return context::Get()->GetBuffer(Uniforms[Index].ResourceHandle);
}
image *uniformGroup::GetTexture(u32 Index)
{
    return context::Get()->GetImage(Uniforms[Index].ResourceHandle);
}
framebuffer *uniformGroup::GetFramebuffer(u32 Index)
{
    return context::Get()->GetFramebuffer(Uniforms[Index].ResourceHandle);
}


#if GFX_API==GFX_VK || GFX_API==GFX_D3D12
accelerationStructure *uniformGroup::GetAccelerationStructure(u32 Index)
{
    return context::Get()->GetAccelerationStructure(Uniforms[Index].ResourceHandle);
}
#endif

void uniformGroup::UpdateBuffer(u32 Index, void *Data, u32 Size, u32 Offset)
{
    gfx::buffer *Buffer = GetBuffer(Index);
    Buffer->CopyData((u8*)Data, Size, Offset);
}


}