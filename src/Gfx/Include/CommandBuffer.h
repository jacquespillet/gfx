#pragma once
#include "RenderPass.h"
#include "Types.h"
#include "Uniform.h"
#include <memory>

namespace gfx
{
struct buffer;
struct bufferInfo
{
    buffer *Resource;
    u32 Offset;
};    

struct image;
struct imageInfo
{
    image* Resource;
    imageUsage::bits Usage = imageUsage::UNKNOWN;
    u32 MipLevel = 0;
    u32 Layer = 0;
};

struct clearColorValues 
{
    f32 R, G, B, A;
};

struct clearDepthStencilValues
{
    f32 Depth;
    u32 Stencil;
};

struct commandBuffer
{
    void Initialize();
    
    void Begin();
    void BeginPass(framebufferHandle Framebuffer, clearColorValues ClearColor, clearDepthStencilValues DepthStencil);
    void BindGraphicsPipeline(pipelineHandle Pipeline);
    void BindComputePipeline(pipelineHandle Pipeline);
    void BindVertexBuffer(vertexBufferHandle Buffer);
    void BindIndexBuffer(bufferHandle Buffer, u32 Offset, indexType IndexType);
    void SetViewport(f32 X, f32 Y, f32 Width, f32 Height);
    void SetScissor(s32 X, s32 Y, u32 Width, u32 Height);
    void DrawArrays(u32 Start, u32 Count, u32 InstanceCount=1);
    void DrawIndexed(u32 Start, u32 Count, u32 InstanceCount=1);
    void Dispatch(u32 NumGroupX, u32 NumGroupY, u32 NumGroupZ);

    void BindUniformGroup(std::shared_ptr<uniformGroup> Group, u32 Binding);
    
    void CopyBufferToImage(const bufferInfo &Source, const imageInfo &Destination);
    void CopyImageToImage(const imageInfo &Source, const imageInfo &Destination);
    void CopyBuffer(const bufferInfo &Source, const bufferInfo &Destination, size_t ByteSize);
    void TransferLayout(const image &Texture, imageUsage::bits OldLayout, imageUsage::bits NewLayout);
    

    void EndPass();
    void End();

    std::shared_ptr<void> ApiData;
};
}