#pragma once

#include "Types.h"
#include <vector>
#include <functional>

namespace app
{
struct window;
}

namespace gfx
{
struct commandBuffer;
struct swapchain;
struct pipeline;

inline void DefaultCallback(const std::string&) {}

struct context
{
    struct initializeInfo
    {
        s32 MajorVersion = 1;
        s32 MinorVersion = 0;
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

    commandBuffer *CreateCommandBuffer();   
    
    swapchain *CreateSwapchain(u32 Width, u32 Height);
    swapchain *RecreateSwapchain(u32 Width, u32 Height, swapchain *OldSwapchain);

    bufferHandle CreateVertexBuffer(f32 *Values, sz Count);
    pipelineHandle CreatePipeline(const char* FileName);   

    renderPassHandle GetDefaultRenderPass();
    
    pipeline *GetPipeline(pipelineHandle Handle);   

    void SubmitCommandBuffer(commandBuffer *CommandBuffer);

    void DestroyCommandBuffer(commandBuffer* Handle);
    void DestroySwapchain(swapchain *Swapchain);

    void *ApiContextData;
};
}