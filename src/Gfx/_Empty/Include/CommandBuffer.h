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
    void SetViewport(float X, float Y, float Width, float Height);
    void ClearColor(float R, float G,float B,float A);
    void ClearBuffers(clearBufferType Type);
    void DrawTriangles(uint32_t Start, uint32_t Count);
    void EndPass();
    void End();
};
}