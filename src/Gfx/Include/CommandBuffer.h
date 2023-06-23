#pragma once
#include "RenderPass.h"
#include "Types.h"

namespace gfx
{
struct buffer;
struct bufferInfo
{
    buffer *Resource;
    u32 Offset;
};    

struct commandBuffer
{
    void Initialize();
    
    void Begin();
    void BeginPass(renderPassHandle RenderPass);
    void BindGraphicsPipeline(pipelineHandle Pipeline);
    void BindVertexBuffer(bufferHandle Buffer);
    void SetViewport(f32 X, f32 Y, f32 Width, f32 Height);
    void ClearColor(f32 R, f32 G,f32 B,f32 A);
    void ClearBuffers(clearBufferType Type);
    void DrawTriangles(uint32_t Start, uint32_t Count);
    
    void CopyBuffer(const bufferInfo &Source, const bufferInfo &Destination, size_t ByteSize);


    void EndPass();
    void End();

    void *ApiData;
};
}