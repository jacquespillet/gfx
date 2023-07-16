#pragma once
#include <vector>
#include <functional>

#include "VertexInput.h"
#include "Types.h"
#include "Pipeline.h"
#include "ResourceManager.h"
#include "Uniform.h"
#include "Image.h"
#include "Buffer.h"
#include <memory>

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
struct framebufferCreateInfo;

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

    static std::shared_ptr<context> Singleton;

    app::window *Window;


    static context *Get();
    static std::shared_ptr<context> Initialize(initializeInfo &InitializeInfo, app::window &Window);

    void StartFrame();
    void EndFrame();
    void Present();

    std::shared_ptr<commandBuffer> CreateCommandBuffer();   
    
    commandBuffer *GetImmediateCommandBuffer();
    std::shared_ptr<commandBuffer> GetCurrentFrameCommandBuffer();
    stageBuffer *GetStageBuffer();
    
    std::shared_ptr<swapchain> CreateSwapchain(u32 Width, u32 Height);
    std::shared_ptr<swapchain> RecreateSwapchain(u32 Width, u32 Height, std::shared_ptr<swapchain> OldSwapchain);

    stageBuffer CreateStageBuffer(sz Size);
    vertexBufferHandle CreateEmptyVertexBuffer();
    bufferHandle CreateBuffer(sz Size, bufferUsage::Bits Usage, memoryUsage MemoryUsage);
    imageHandle CreateImage(const imageData &ImageData, const imageCreateInfo& CreateInfo);
    imageHandle CreateImage(u32 Width, u32 Height, format Format, u8 *Pixels);

    //TODO: return a technique struct with multiple passes and pipelines
    // struct technique
    // {
    //     std::vector<pipelineHandle> Passes;
    // }
    
    pipelineHandle CreatePipelineFromFile(const char *FileName, framebufferHandle Framebuffer = InvalidHandle); 
    pipelineHandle CreatePipeline(const pipelineCreation &PipelineCreation);


    renderPassHandle GetDefaultRenderPass();
    framebufferHandle GetSwapchainFramebuffer();

    framebufferHandle CreateFramebuffer(const framebufferCreateInfo &CreateInfo);
    
    pipeline *GetPipeline(pipelineHandle Handle);   

    void SubmitCommandBuffer(commandBuffer *CommandBuffer);
    void SubmitCommandBufferImmediate(commandBuffer *CommandBuffer);
    void BindUniformsToPipeline(std::shared_ptr<uniformGroup> Uniforms, pipelineHandle PipelineHandle, u32 Binding);

    void CopyDataToBuffer(bufferHandle BufferHandle, void *Ptr, sz Size, sz Offset);

    void DestroyCommandBuffer(commandBuffer* Handle);
    void DestroySwapchain(swapchain *Swapchain);
    void DestroyPipeline(pipelineHandle Pipeline);
    void DestroyFramebuffer(framebufferHandle Framebuffer);
    void DestroyBuffer(bufferHandle Buffer);
    void DestroyVertexBuffer(vertexBufferHandle Buffer);
    void DestroyImage(imageHandle Buffer);
    void DestroySwapchain();


    void Cleanup();
    void WaitIdle();

    void OnResize(u32 newWidth, u32 NewHeight);

    std::shared_ptr<void> ApiContextData;

    resourceManager ResourceManager;

    renderPassOutput SwapchainOutput;

    std::shared_ptr<swapchain> Swapchain;
};
}