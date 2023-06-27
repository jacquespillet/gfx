#pragma once
#include <vector>
#include <functional>

#include "Types.h"
#include "Pipeline.h"
#include "ResourceManager.h"

namespace app
{
struct window;
}

namespace gfx
{
struct commandBuffer;
struct swapchain;
struct pipeline;
struct stageBuffer;

inline void DefaultCallback(const std::string&) {}

struct context
{
    struct initializeInfo
    {
        s32 MajorVersion = 1;
        s32 MinorVersion = 2;
        std::function<void(const std::string&)> ErrorCallback = DefaultCallback;
        std::function<void(const std::string&)> InfoCallback = DefaultCallback;

        std::vector<const char *> Extensions;
        std::vector<const char *> Layers;

        const char *AppName = "DefaultApp";
        const char *EngineName = "DefaultEngine";

        deviceType PreferredDeviceType = deviceType::DISCRETE_GPU;
        std::vector<const char *> DeviceExtensions;
        u64 VirtualFrameCount = 3;
        u64 MaxStageBufferSize = 64 * 1024 * 1024;

        b8 Debug=true;    
    };

    static context* Singleton;


    static context *Get();
    static context* Initialize(initializeInfo &InitializeInfo, app::window &Window);

    void StartFrame();
    void EndFrame();
    void Present();

    commandBuffer *CreateCommandBuffer();   
    
    commandBuffer *GetImmediateCommandBuffer();
    commandBuffer *GetCurrentFrameCommandBuffer();
    stageBuffer *GetStageBuffer();
    
    swapchain *CreateSwapchain(u32 Width, u32 Height);
    swapchain *RecreateSwapchain(u32 Width, u32 Height, swapchain *OldSwapchain);

    stageBuffer CreateStageBuffer(sz Size);
    bufferHandle CreateVertexBuffer(f32 *Values, sz Count);
    bufferHandle CreateBuffer(sz Size, bufferUsage::Bits Usage, memoryUsage MemoryUsage);

    //TODO: return a technique struct with multiple passes and pipelines
    // struct technique
    // {
    //     std::vector<pipelineHandle> Passes;
    // }
    pipelineHandle CreatePipelineFromFile(const char *FileName); 
    pipelineHandle CreatePipeline(const pipelineCreation &PipelineCreation);

    imageHandle CreateImage(u32 Width, u32 Height, format Format, u8 *Pixels);

    renderPassHandle GetDefaultRenderPass();
    framebufferHandle GetSwapchainFramebuffer();
    
    pipeline *GetPipeline(pipelineHandle Handle);   

    void SubmitCommandBuffer(commandBuffer *CommandBuffer);
    void SubmitCommandBufferImmediate(commandBuffer *CommandBuffer);

    void DestroyCommandBuffer(commandBuffer* Handle);
    void DestroySwapchain(swapchain *Swapchain);

    void *ApiContextData;

    resourceManager ResourceManager;

    renderPassOutput SwapchainOutput;

    swapchain *Swapchain;
};
}