#include "../../App/App.h"
#include "../Include/Context.h"
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
#include "GLImage.h"
#include "GLFramebuffer.h"

#include <glad/gl.h>

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
    
    int version = gladLoadGL(glfwGetProcAddress);
    if (version == 0) {
        printf("Failed to initialize OpenGL context\n");
        exit(0);
    }    

    GLData->CommandBuffer = std::make_shared<commandBuffer>();
    GLData->CommandBuffer->Initialize();

    if(InitializeInfo.EnableMultisampling)
        glEnable(GL_MULTISAMPLE);

    

    return Singleton;
}


bufferHandle context::CreateBuffer(sz Size, bufferUsage::value Usage, memoryUsage MemoryUsage, sz Stride)
{
    bufferHandle Handle = ResourceManager.Buffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }
    buffer *Buffer = GetBuffer(Handle);
    Buffer->ApiData = std::make_shared<glBuffer>();
    
    Buffer->Init(Size, Stride, Usage, MemoryUsage);
    return Handle;
}

void context::CopyDataToBuffer(bufferHandle BufferHandle, void *Ptr, sz Size, sz Offset)
{
    buffer *Buffer = GetBuffer(BufferHandle);
    Buffer->CopyData((u8*)Ptr, Size, Offset);
}

void context::BindUniformsToPipeline(std::shared_ptr<uniformGroup> Uniforms, pipelineHandle PipelineHandle, u32 Binding, b8 Force){

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
    framebuffer *Framebuffer = GetFramebuffer(FramebufferHandle);
    Framebuffer->Width = CreateInfo.Width;
    Framebuffer->Height = CreateInfo.Height;
    Framebuffer->ApiData = std::make_shared<glFramebufferData>();
    
    GET_API_DATA(GLFramebufferData, glFramebufferData, Framebuffer);

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
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, GLFramebufferData->ColorTextures[i], 0);

        ColorAttachments[i] = (GLenum)(GL_COLOR_ATTACHMENT0 + i);
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

    glDrawBuffers((GLsizei)ColorAttachments.size(), ColorAttachments.data());

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        assert(false);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return FramebufferHandle;
}

imageHandle context::CreateImage(imageData &ImageData, const imageCreateInfo& CreateInfo)
{
    imageHandle ImageHandle = ResourceManager.Images.ObtainResource();
    if(ImageHandle == InvalidHandle)
    {
        return ImageHandle;
    }
    image *Image = GetImage(ImageHandle);
    *Image = image();
    Image->Init(ImageData, CreateInfo);
    return ImageHandle;
}

imageHandle context::CreateImageArray(u32 Width, u32 Height, u32 Depth, format Format, imageUsage::bits Usage)
{
    imageHandle ImageHandle = ResourceManager.Images.ObtainResource();
    if(ImageHandle == InvalidHandle)
    {
        return ImageHandle;
    }
    image *Image = GetImage(ImageHandle);
    *Image = image();
    Image->InitAsArray(Width, Height, Depth, Format, imageUsage::SHADER_READ, memoryUsage::GpuOnly);
    return ImageHandle;
}



imageHandle context::CreateImageCubemap(const imageData &Left, const imageData &Right, const imageData &Top, const imageData &Bottom, const imageData &Back, const imageData &Front, const imageCreateInfo& CreateInfo)
{
    imageHandle ImageHandle = ResourceManager.Images.ObtainResource();
    if(ImageHandle == InvalidHandle)
    {
        return ImageHandle;
    }
    image *Image = GetImage(ImageHandle);
    *Image = image();
    Image->InitAsCubemap(Left, Right, Top, Bottom, Back, Front, CreateInfo);
    return ImageHandle;
}


std::shared_ptr<swapchain> context::CreateSwapchain(u32 Width, u32 Height, std::shared_ptr<swapchain> OldSwapchain)
{
    GET_CONTEXT(GLData, this);    
    
    //Create Framebuffer
    GLData->SwapchainFramebuffer = Singleton->ResourceManager.Framebuffers.ObtainResource();
    if(GLData->SwapchainFramebuffer == InvalidHandle)
    {
        assert(false);
    }
    framebuffer *SwapchainFramebuffer = Singleton->GetFramebuffer(GLData->SwapchainFramebuffer);
    SwapchainFramebuffer->ApiData = std::make_shared<glFramebufferData>();

    GET_API_DATA(GLFramebufferData, glFramebufferData, SwapchainFramebuffer);
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

vertexBufferHandle context::CreateVertexBuffer(const vertexBufferCreateInfo &CreateInfo)
{
    GET_CONTEXT(GLData, this);

    vertexBufferHandle Handle = this->ResourceManager.VertexBuffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        assert(false);
        return Handle;
    }
    

    vertexBuffer *VertexBuffer = GetVertexBuffer(Handle);
    VertexBuffer->NumVertexStreams = CreateInfo.NumVertexStreams;
    memcpy(&VertexBuffer->VertexStreams[0], &CreateInfo.VertexStreams[0], commonConstants::MaxVertexStreams * sizeof(vertexStreamData));
    
    VertexBuffer->ApiData = std::make_shared<glVertexBuffer>();
    GET_API_DATA(GLVertexBuffer, glVertexBuffer, VertexBuffer);

    //Create the vao
    glGenVertexArrays(1, &GLVertexBuffer->VAO);

    glBindVertexArray(GLVertexBuffer->VAO);
    //Create the vbos
    for(sz i=0; i<VertexBuffer->NumVertexStreams; i++)
    {
        GLVertexBuffer->VertexBuffers[i] = context::Get()->ResourceManager.Buffers.ObtainResource();
        buffer *Buffer = context::Get()->GetBuffer(GLVertexBuffer->VertexBuffers[i]);
        Buffer->ApiData = std::make_shared<glBuffer>();

        Buffer->Init(VertexBuffer->VertexStreams[i].Size, 1, bufferUsage::VertexBuffer, memoryUsage::GpuOnly);
        GLData->CheckErrors();
        Buffer->CopyData((u8*)VertexBuffer->VertexStreams[i].Data, VertexBuffer->VertexStreams[i].Size, 0);
        GLData->CheckErrors();
        GET_API_DATA(GLBuffer, glBuffer, Buffer);

        glBindBuffer(GLBuffer->Target, GLBuffer->Handle);
        
        u32 StartPtr=0;
        for(u32 j=0; j<VertexBuffer->VertexStreams[i].AttributesCount; j++)
        {
            glVertexAttribPointer(VertexBuffer->VertexStreams[i].InputAttributes[j].InputIndex, 
                                VertexBuffer->VertexStreams[i].InputAttributes[j].ElementCount, 
                                VertexAttributeTypeToNative(VertexBuffer->VertexStreams[i].InputAttributes[j].Type), 
                                VertexBuffer->VertexStreams[i].InputAttributes[j].Normalized,
                                (GLsizei)VertexBuffer->VertexStreams[i].Stride, 
                                (void*)((uintptr_t)StartPtr));
            glEnableVertexAttribArray(VertexBuffer->VertexStreams[i].InputAttributes[j].InputIndex);
            if(VertexBuffer->VertexStreams[i].InputRate == vertexInputRate::PerInstance) glVertexAttribDivisor(VertexBuffer->VertexStreams[i].InputAttributes[j].InputIndex, 1); 
            StartPtr += VertexBuffer->VertexStreams[i].InputAttributes[j].ElementCount * VertexBuffer->VertexStreams[i].InputAttributes[j].ElementSize;
        }
        glBindBuffer(GLBuffer->Target, 0);
    }
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

    framebuffer *SwapchainFramebuffer = Singleton->GetFramebuffer(GLData->SwapchainFramebuffer);
    SwapchainFramebuffer->ApiData = std::make_shared<glFramebufferData>();
    GET_API_DATA(GLFramebufferData, glFramebufferData, SwapchainFramebuffer);
    
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
    GET_CONTEXT(GLData, this);
    GLData->CheckErrors();
}

pipelineHandle context::RecreatePipeline(const pipelineCreation &PipelineCreation, pipelineHandle PipelineHandle)
{
    GET_CONTEXT(GLData, this);
    pipeline *Pipeline = GetPipeline(PipelineHandle);
    Pipeline->Name = std::string(PipelineCreation.Name);
    Pipeline->Creation = PipelineCreation;    
    GET_API_DATA(GLPipelineData, glPipeline, Pipeline);
    GLPipelineData->DestroyGLResources();
    GLPipelineData->Create(PipelineCreation);
    return PipelineHandle;
}

pipelineHandle context::CreatePipeline(const pipelineCreation &PipelineCreation)
{
    GET_CONTEXT(GLData, this);

    pipelineHandle Handle = ResourceManager.Pipelines.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }
    pipeline *Pipeline = GetPipeline(Handle);
    Pipeline->ApiData = std::make_shared<glPipeline>();
    GET_API_DATA(GLPipeline, glPipeline, Pipeline);
    //Compile and link shaders
    GLPipeline->Create(PipelineCreation);
    
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

void context::DestroyPipeline(pipelineHandle PipelineHandle)
{
    GET_CONTEXT(GLData, this);    
    pipeline *Pipeline = GetPipeline(PipelineHandle);
    GET_API_DATA(GLPipelineData, glPipeline, Pipeline);

    if(GLPipelineData->ShaderProgram->VertexShader != nullptr) glDeleteShader(GLPipelineData->ShaderProgram->VertexShader->ShaderObject);
    if(GLPipelineData->ShaderProgram->GeometryShader != nullptr) glDeleteShader(GLPipelineData->ShaderProgram->GeometryShader->ShaderObject);
    if(GLPipelineData->ShaderProgram->FragmentShader != nullptr) glDeleteShader(GLPipelineData->ShaderProgram->FragmentShader->ShaderObject);
    if(GLPipelineData->ShaderProgram->ComputeShader != nullptr) glDeleteShader(GLPipelineData->ShaderProgram->ComputeShader->ShaderObject);

    glDeleteProgram(GLPipelineData->ShaderProgram->ProgramShaderObject);

    ResourceManager.Pipelines.ReleaseResource(PipelineHandle);


}

void context::DestroyBuffer(bufferHandle BufferHandle)
{
    buffer *Buffer = GetBuffer(BufferHandle);
    GET_API_DATA(GLBuffer, glBuffer, Buffer);

    glDeleteBuffers(1, &GLBuffer->Handle);
    ResourceManager.Buffers.ReleaseResource(BufferHandle);
}

void context::DestroyVertexBuffer(vertexBufferHandle VertexBufferHandle)
{
    vertexBuffer *VertexBuffer = GetVertexBuffer(VertexBufferHandle);
    for (sz i = 0; i < VertexBuffer->NumVertexStreams; i++)
    {
        DestroyBuffer(VertexBuffer->VertexStreams[i].Buffer);
    }
    
    GET_API_DATA(GLVertexBuffer, glVertexBuffer, VertexBuffer);
    glDeleteVertexArrays(1, &GLVertexBuffer->VAO);

    ResourceManager.VertexBuffers.ReleaseResource(VertexBufferHandle);
}

void context::DestroyImage(imageHandle ImageHandle)
{
    image *Image = GetImage(ImageHandle);
    if(!Image) return;
    GET_API_DATA(GLImage, glImage, Image);
    glDeleteTextures(1, &GLImage->Handle);
    ResourceManager.Images.ReleaseResource(ImageHandle);
}

void context::DestroySwapchain()
{
    GET_CONTEXT(GLData, this);    
    ResourceManager.Framebuffers.ReleaseResource(GLData->SwapchainFramebuffer);
}

void context::DestroyFramebuffer(framebufferHandle FramebufferHandle)
{
    framebuffer *Framebuffer = GetFramebuffer(FramebufferHandle);
    GET_API_DATA(GLFramebuffer, glFramebufferData, Framebuffer)
    
    glDeleteTextures((GLsizei)GLFramebuffer->ColorTextures.size(), GLFramebuffer->ColorTextures.data());
    glDeleteTextures(1, &GLFramebuffer->DepthTexture);
    glDeleteFramebuffers(1, &GLFramebuffer->Handle);
 
    ResourceManager.Framebuffers.ReleaseResource(FramebufferHandle);
}

void context::Cleanup()
{
    ResourceManager.Destroy();
}

void context::WaitIdle()
{
}


}