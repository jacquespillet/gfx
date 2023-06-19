#pragma once
#include "RenderTarget.h"
#include "Types.h"
namespace gfx
{
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
    void EndPass();
    void End();

    void *VkData;
};
}