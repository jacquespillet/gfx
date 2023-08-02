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
struct shader;

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

        b8 EnableMultisampling=false;

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
    
    std::shared_ptr<swapchain> CreateSwapchain(u32 Width, u32 Height, std::shared_ptr<swapchain> OldSwapchain=nullptr);
    std::shared_ptr<swapchain> RecreateSwapchain(u32 Width, u32 Height, std::shared_ptr<swapchain> OldSwapchain);

    stageBuffer CreateStageBuffer(sz Size);
    vertexBufferHandle CreateVertexBuffer(const vertexBufferCreateInfo &CreateInfo);
   
    bufferHandle CreateBuffer(sz Size, bufferUsage::value Usage, memoryUsage MemoryUsage, sz Stride = 0);
    imageHandle CreateImage(const imageData &ImageData, const imageCreateInfo& CreateInfo);
    imageHandle CreateImageCubemap(const imageData &Left, const imageData &Right, const imageData &Top, const imageData &Bottom, const imageData &Back, const imageData &Front, const imageCreateInfo& CreateInfo);
    imageHandle CreateImage(u32 Width, u32 Height, format Format, u8 *Pixels);

    pipelineHandle CreatePipelineFromFile(const char *FileName, framebufferHandle Framebuffer = InvalidHandle); 
    pipelineHandle CreatePipeline(const pipelineCreation &PipelineCreation);
    pipelineHandle RecreatePipeline(const pipelineCreation &PipelineCreation, pipelineHandle PipelineHandle);

    renderPassHandle CreateRenderPass(const renderPassOutput &Output);

    renderPassHandle GetDefaultRenderPass();
    framebufferHandle GetSwapchainFramebuffer();

    framebufferHandle CreateFramebuffer(const framebufferCreateInfo &CreateInfo);
    
    void SubmitCommandBuffer(commandBuffer *CommandBuffer);
    void SubmitCommandBufferImmediate(commandBuffer *CommandBuffer);
    void BindUniformsToPipeline(std::shared_ptr<uniformGroup> Uniforms, pipelineHandle PipelineHandle, u32 Binding, b8 Force = false);

    void CopyDataToBuffer(bufferHandle BufferHandle, void *Ptr, sz Size, sz Offset);


    void DestroyCommandBuffer(commandBuffer* Handle);
    void DestroySwapchain(swapchain *Swapchain);
    void DestroyPipeline(pipelineHandle Pipeline);
    void DestroyFramebuffer(framebufferHandle Framebuffer);
    void DestroyBuffer(bufferHandle Buffer);
    void DestroyVertexBuffer(vertexBufferHandle Buffer);
    void DestroyImage(imageHandle Buffer);
    void DestroySwapchain();

    struct resourceDeletion
    {
        u32 Handle;
        enum class type 
        {
            Buffer,
            VertexBuffer,
            Image,
            Pipeline,
            Framebuffer
        } Type;
    };
    std::vector<resourceDeletion> ResourceDeletionQueue;
    void QueueDestroyBuffer(bufferHandle Buffer);
    void QueueDestroyVertexBuffer(vertexBufferHandle Buffer);
    void QueueDestroyPipeline(pipelineHandle Pipeline);
    void QueueDestroyImage(imageHandle Image);
    void ProcessDeletionQueue();

    buffer *GetBuffer(bufferHandle Handle);
    vertexBuffer *GetVertexBuffer(vertexBufferHandle Handle);
    image *GetImage(imageHandle Handle);
    pipeline *GetPipeline(pipelineHandle Handle);
    shader *GetShader(shaderStateHandle Handle);
    renderPass *GetRenderPass(renderPassHandle Handle);
    framebuffer *GetFramebuffer(framebufferHandle Handle);


    void Cleanup();
    void WaitIdle();

    void OnResize(u32 newWidth, u32 NewHeight);

    std::shared_ptr<void> ApiContextData;

    resourceManager ResourceManager;

    renderPassOutput SwapchainOutput;
    renderPassHandle SwapchainRenderPass;

    std::shared_ptr<swapchain> Swapchain;
    
    u32 MultiSampleCount=1;
};
}