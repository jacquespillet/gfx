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
#include "GLPipeline.h"
#include "GLCommon.h"
#include "GLBuffer.h"
#include "GLMapping.h"
#include "GLFramebuffer.h"

#include <GL/glew.h>

#include <iostream>

namespace gfx
{

std::shared_ptr<context> context::Singleton = {};

void glData::CheckErrors()
{
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        switch (err)
        {
            case GL_INVALID_ENUM:
                std::cout << "GL_INVALID_ENUM " << std::endl;
                break; 
            case GL_INVALID_VALUE:
                std::cout << "GL_INVALID_VALUE " << std::endl;
                break; 
            case GL_INVALID_OPERATION:
                std::cout << "GL_INVALID_OPERATION " << std::endl;
                break; 
            case GL_STACK_OVERFLOW:
                std::cout << "GL_STACK_OVERFLOW " << std::endl;
                break; 
            case GL_STACK_UNDERFLOW:
                std::cout << "GL_STACK_UNDERFLOW " << std::endl;
                break; 
            case GL_OUT_OF_MEMORY:
                std::cout << "GL_OUT_OF_MEMORY " << std::endl;
                break; 
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                std::cout << "GL_INVALID_FRAMEBUFFER_OPERATION " << std::endl;
                break; 
            case GL_CONTEXT_LOST:
                std::cout << "GL_CONTEXT_LOST " << std::endl;
                break; 
        }      
    }
}

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

    GLData->CommandBuffer = std::make_shared<commandBuffer>();
    GLData->CommandBuffer->Initialize();

    return Singleton;
}


// stageBuffer context::CreateStageBuffer(sz Size)
// {
//     stageBuffer Result;
//     // Result.Init(Size);
//     return Result;
// }

bufferHandle context::CreateBuffer(sz Size, bufferUsage::Bits Usage, memoryUsage MemoryUsage)
{
    bufferHandle Handle = ResourceManager.Buffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }
    buffer *Buffer = (buffer*)ResourceManager.Buffers.GetResource(Handle);
    Buffer->ApiData = std::make_shared<glBuffer>();
    std::shared_ptr<glBuffer> D12BufferData = std::static_pointer_cast<glBuffer>(Buffer->ApiData);
    
    Buffer->Init(Size, Usage, MemoryUsage);
    return Handle;
}

void context::BindUniformsToPipeline(std::shared_ptr<uniformGroup> Uniforms, pipelineHandle PipelineHandle, u32 Binding){

}

framebufferHandle context::CreateFramebuffer(const framebufferCreateInfo &CreateInfo)
{
    //Create the render pass
    GET_CONTEXT(GLData, this);
    
    framebufferHandle FramebufferHandle = ResourceManager.Framebuffers.ObtainResource();
    if(FramebufferHandle == InvalidHandle)
    {
        assert(false);
        return InvalidHandle;
    }
    framebuffer *Framebuffer = (framebuffer*)ResourceManager.Framebuffers.GetResource(FramebufferHandle);
    Framebuffer->Width = CreateInfo.Width;
    Framebuffer->Height = CreateInfo.Height;
    Framebuffer->ApiData = std::make_shared<glFramebufferData>();
    std::shared_ptr<glFramebufferData> GLFramebufferData = std::static_pointer_cast<glFramebufferData>(Framebuffer->ApiData);

    GLFramebufferData->ColorTextures.resize(CreateInfo.ColorFormats.size());

    glGenFramebuffers(1, &GLFramebufferData->Handle);
    glBindFramebuffer(GL_FRAMEBUFFER, GLFramebufferData->Handle);
    std::vector<GLenum> ColorAttachments(CreateInfo.ColorFormats.size());
    for(sz i=0; i<CreateInfo.ColorFormats.size(); i++)
    {
        glGenTextures(1, &GLFramebufferData->ColorTextures[i]);
        glBindTexture(GL_TEXTURE_2D, GLFramebufferData->ColorTextures[i]);
        // Set texture parameters and allocate storage
        glTexImage2D(GL_TEXTURE_2D, 0, FormatToNativeInternal(CreateInfo.ColorFormats[i]), CreateInfo.Width, CreateInfo.Height, 0, FormatToNative(CreateInfo.ColorFormats[i]), FormatToType(CreateInfo.ColorFormats[i]), nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, GLFramebufferData->ColorTextures[i], 0);

        ColorAttachments[i] = GL_COLOR_ATTACHMENT0 + i;
    }

#if 0 // No depth texture
    glGenRenderbuffers(1, &GLFramebufferData->DepthTexture);
    glBindRenderbuffer(GL_RENDERBUFFER, GLFramebufferData->DepthTexture);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, CreateInfo.Width, CreateInfo.Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, GLFramebufferData->DepthTexture);
#else
    glGenTextures(1, &GLFramebufferData->DepthTexture);
    glBindTexture(GL_TEXTURE_2D, GLFramebufferData->DepthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, FormatToNativeInternal(CreateInfo.DepthFormat), CreateInfo.Width, CreateInfo.Height, 0, FormatToNative(CreateInfo.DepthFormat), FormatToType(CreateInfo.DepthFormat), nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, GLFramebufferData->DepthTexture, 0);
#endif

    glDrawBuffers(ColorAttachments.size(), ColorAttachments.data());

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        assert(false);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::shared_ptr<swapchain> context::CreateSwapchain(u32 Width, u32 Height)
{
    GET_CONTEXT(GLData, this);    
    
    //Create Framebuffer
    GLData->SwapchainFramebuffer = Singleton->ResourceManager.Framebuffers.ObtainResource();
    if(GLData->SwapchainFramebuffer == InvalidHandle)
    {
        assert(false);
    }
    framebuffer *SwapchainFramebuffer = (framebuffer*)Singleton->ResourceManager.Framebuffers.GetResource(GLData->SwapchainFramebuffer);
    SwapchainFramebuffer->ApiData = std::make_shared<glFramebufferData>();
    std::shared_ptr<glFramebufferData> GLFramebufferData = std::static_pointer_cast<glFramebufferData>(SwapchainFramebuffer->ApiData);
    
    GLint FramebufferHandle;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &FramebufferHandle);

    GLFramebufferData->Handle = (GLuint)FramebufferHandle;
    SwapchainFramebuffer->Width = Width;
    SwapchainFramebuffer->Height = Height;

    std::shared_ptr<swapchain> Swapchain = std::make_shared<swapchain>();
    Swapchain->Width = Width;
    Swapchain->Height = Height;
    return Swapchain;
}

bufferHandle context::CreateVertexBuffer(f32 *Values, sz ByteSize, sz Stride, const std::vector<vertexInputAttribute> &Attributes)
{
    GET_CONTEXT(GLData, this);
    bufferHandle Handle = ResourceManager.Buffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }
    buffer *Buffer = (buffer*)ResourceManager.Buffers.GetResource(Handle);
    Buffer->ApiData = std::make_shared<glBuffer>();
    Buffer->Init(ByteSize, bufferUsage::VertexBuffer, memoryUsage::GpuOnly);
    GLData->CheckErrors();
    
    Buffer->CopyData((u8*)Values, ByteSize, 0);
    GLData->CheckErrors();

    std::shared_ptr<glBuffer> GLBuffer = std::static_pointer_cast<glBuffer>(Buffer->ApiData);
    

    glBindVertexArray(GLBuffer->VAO);
    glBindBuffer(GLBuffer->Target, GLBuffer->Handle);
    sz StartPtr=0;
    for(int i=0; i<Attributes.size(); i++)
    {
        glEnableVertexAttribArray(i);
        glVertexAttribPointer(i, 
                              Attributes[i].ElementCount, 
                              VertexAttributeTypeToNative(Attributes[i].Type), 
                              Attributes[i].Normalized,
                              Stride, 
                              (void*)((uintptr_t)StartPtr));
        StartPtr += Attributes[i].ElementCount * Attributes[i].ElementSize;
    }    
    glBindBuffer(GLBuffer->Target, 0);
    glBindVertexArray(0);
    GLData->CheckErrors();

    return Handle;
}

renderPassHandle context::GetDefaultRenderPass()
{
    return 0;
}

std::shared_ptr<commandBuffer> context::GetCurrentFrameCommandBuffer()
{
    GET_CONTEXT(GLData, this);
    return GLData->CommandBuffer;
}

framebufferHandle context::GetSwapchainFramebuffer()
{
    GET_CONTEXT(GLData, this);

    framebuffer *SwapchainFramebuffer = (framebuffer*)Singleton->ResourceManager.Framebuffers.GetResource(GLData->SwapchainFramebuffer);
    SwapchainFramebuffer->ApiData = std::make_shared<glFramebufferData>();
    std::shared_ptr<glFramebufferData> GLFramebufferData = std::static_pointer_cast<glFramebufferData>(SwapchainFramebuffer->ApiData);
    return GLFramebufferData->Handle;
}

void context::StartFrame()
{
}

void context::EndFrame()
{
    GET_CONTEXT(GLData, this);
    GLData->CommandBuffer->End();
}

void context::Present()
{
    this->Window->Present();
}

// void context::SubmitCommandBufferImmediate(commandBuffer *CommandBuffer)
// {
    
// }

pipelineHandle context::CreatePipeline(const pipelineCreation &PipelineCreation)
{
    GET_CONTEXT(GLData, this);

    pipelineHandle Handle = ResourceManager.Pipelines.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }
    pipeline *Pipeline = (pipeline*)ResourceManager.Pipelines.GetResource(Handle);
    Pipeline->ApiData = std::make_shared<glPipeline>();
    std::shared_ptr<glPipeline> GLPipeline = std::static_pointer_cast<glPipeline>(Pipeline->ApiData);

    //Compile and link shaders

    GLPipeline->ShaderProgram = std::make_shared<glShaderProgram>();
    for (size_t i = 0; i < PipelineCreation.Shaders.StagesCount; i++)
    {
        GLPipeline->ShaderProgram->SetCodeForStage(PipelineCreation.Shaders.Stages[i].Code, PipelineCreation.Shaders.Stages[i].Stage);
    }
    GLPipeline->ShaderProgram->Compile();
    
    GLData->CheckErrors();
    //Blend state

    //Depth stencil state

    //Multisample state

    //Rasterizer state

    //Viewport state

    //Scissor state


    //Render targets


    return Handle;
}

void context::OnResize(u32 NewWidth, u32 NewHeight)
{
    
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