#pragma once

#include "Types.h"

namespace gfx
{
struct commandBuffer;
struct swapchain;
struct pipeline;

struct context
{
    void Initialize();
    commandBuffer *CreateCommandBuffer();   
    swapchain *CreateSwapchain();
    bufferHandle CreateVertexBuffer(float *Values, size_t Count);
    pipelineHandle CreatePipeline(const char *FileName);   

    renderPassHandle GetDefaultRenderPass();
    
    pipeline *GetPipeline(pipelineHandle Handle);   

    void SubmitCommandBuffer(commandBuffer *CommandBuffer);

    void DestroyCommandBuffer(commandBuffer* Handle);
    void DestroySwapchain(swapchain *Swapchain);
};
}