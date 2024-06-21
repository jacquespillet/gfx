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
struct accelerationStructure;

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

#if GFX_API == GFX_VK || GFX_API == GFX_D3D12
        b8 EnableRTX=false;    
#endif
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
    imageHandle CreateImage(imageData &ImageData, const imageCreateInfo& CreateInfo);
    imageHandle CreateImageCubemap(const imageData &Left, const imageData &Right, const imageData &Top, const imageData &Bottom, const imageData &Back, const imageData &Front, const imageCreateInfo& CreateInfo);
    imageHandle CreateImage(u32 Width, u32 Height, format Format, imageUsage::bits ImageUsage, memoryUsage MemoryUsage, u8 *Pixels, b8 GenerateMips=false);
    imageHandle CreateImageArray(u32 Width, u32 Height, u32 Depth, format Format, imageUsage::bits Usage);

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

    bool RTXEnabled=false;
#if GFX_API == GFX_VK || GFX_API == GFX_D3D12
    accelerationStructureHandle CreateBLAccelerationStructure(uint32_t NumVertices, uint32_t Stride, gfx::format Format, bufferHandle VertexBufferHandle, gfx::indexType IndexType = gfx::indexType::Uint16, uint32_t NumTriangles = 0, bufferHandle IndexBufferHandle = InvalidHandle, uint32_t PositionOffset=0);
    accelerationStructureHandle CreateTLAccelerationStructure(std::vector<glm::mat4> &Transforms, std::vector<accelerationStructureHandle> &AccelerationStructures, std::vector<int> Instances);
    // void CreateAccelerationStructure()
#endif


    void DestroyCommandBuffer(commandBuffer* Handle);
    void DestroyAccelerationStructure(accelerationStructureHandle Handle);
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

#if GFX_API==GFX_VK || GFX_API==GFX_D3D12
    accelerationStructure *GetAccelerationStructure(accelerationStructureHandle Handle);
#endif

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