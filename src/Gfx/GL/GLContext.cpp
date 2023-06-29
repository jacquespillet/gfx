#include "../../App/App.h"
#include "../Include/GfxContext.h"
#include "../Include/Image.h"
#include "../Include/CommandBuffer.h"
#include "../Include/Swapchain.h"
#include "../Include/Buffer.h"
#include "../Include/Pipeline.h"
#include "../Include/Shader.h"
#include "../Include/Framebuffer.h"
#include "../Include/RenderPass.h"
#include "../Include/Framebuffer.h"
#include "../Include/Memory.h"
#include "../Common/Util.h"

#include "GLContext.h"
#include "GLCommon.h"

#include <GL/glew.h>

#include <iostream>

namespace gfx
{

std::shared_ptr<context> context::Singleton = {};

context *context::Get()
{
    return Singleton.get();
}


std::shared_ptr<context> context::Initialize(initializeInfo &InitializeInfo, app::window &Window)
{
    if(Singleton==nullptr){
        Singleton = std::make_shared<context>();
    }

    Singleton->ResourceManager.Init();
    Singleton->ApiContextData = std::make_shared<glData>();
    GET_CONTEXT(GLData, Singleton);
    
    Singleton->Window = &Window;
    
    bool Error = glewInit() != GLEW_OK;
    if(Error)
    {
        fprintf(stderr, "Failed to initialize OpenGL Loader!\n");
        assert(false);
    }
    
    return Singleton;
}


// stageBuffer context::CreateStageBuffer(sz Size)
// {
//     stageBuffer Result;
//     // Result.Init(Size);
//     return Result;
// }

// bufferHandle context::CreateBuffer(sz Size, bufferUsage::Bits Usage, memoryUsage MemoryUsage)
// {
//     return 0;
// }


// std::shared_ptr<swapchain> context::CreateSwapchain(u32 Width, u32 Height)
// {
//     GET_CONTEXT(D12Data, this);    
//     std::shared_ptr<swapchain> Swapchain = std::make_shared<swapchain>();
//     Swapchain->Width = Width;
//     Swapchain->Height = Height;
//     return Swapchain;
// }

// bufferHandle context::CreateVertexBuffer(f32 *Values, sz ByteSize, sz Stride)
// {
//     GET_CONTEXT(D12Data, this);
//     bufferHandle Handle = ResourceManager.Buffers.ObtainResource();
//     if(Handle == InvalidHandle)
//     {
//         return Handle;
//     }

//     return Handle;
// }


// void context::SubmitCommandBufferImmediate(commandBuffer *CommandBuffer)
// {
    
// }

pipelineHandle context::CreatePipeline(const pipelineCreation &PipelineCreation)
{
    pipelineHandle Handle = ResourceManager.Pipelines.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }
    pipeline *Pipeline = (pipeline*)ResourceManager.Pipelines.GetResource(Handle);
   
    return Handle;
}

// renderPassHandle context::GetDefaultRenderPass()
// {
//     return 0;
// }

// framebufferHandle context::GetSwapchainFramebuffer()
// {
//     return 0;
// }



// // std::shared_ptr<commandBuffer> context::GetCurrentFrameCommandBuffer()
// // {
// //     GET_CONTEXT(D12Data, this);
// //     return D12Data->VirtualFrames.CommandBuffer;
// // }

// void context::Present()
// {

// }

// void context::EndFrame()
// {
// }

// void context::StartFrame()
// {
// }

// void context::DestroyPipeline(pipelineHandle PipelineHandle)
// {

// }
// void context::DestroyBuffer(bufferHandle BufferHandle)
// {

// }
// void context::DestroySwapchain()
// {

// }

// void context::WaitIdle()
// {
// }

// void context::Cleanup()
// {

// }

}