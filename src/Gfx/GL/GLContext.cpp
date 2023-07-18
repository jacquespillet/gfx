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
#include "GLImage.h"
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

    if(InitializeInfo.EnableMultisampling)
        glEnable(GL_MULTISAMPLE);

    return Singleton;
}


bufferHandle context::CreateBuffer(sz Size, bufferUsage::value Usage, memoryUsage MemoryUsage)
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

//TODO: Use glBufferData if memory is gpu only
void context::CopyDataToBuffer(bufferHandle BufferHandle, void *Ptr, sz Size, sz Offset)
{
    buffer *Buffer = (buffer*)ResourceManager.Buffers.GetResource(BufferHandle);
    
    Buffer->Name = "";
    std::shared_ptr<glBuffer> GLBuffer = std::static_pointer_cast<glBuffer>(Buffer->ApiData);

    Buffer->CopyData((u8*)Ptr, Size, Offset);
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

imageHandle context::CreateImage(const imageData &ImageData, const imageCreateInfo& CreateInfo)
{
    imageHandle ImageHandle = ResourceManager.Images.ObtainResource();
    if(ImageHandle == InvalidHandle)
    {
        return ImageHandle;
    }
    image *Image = (image*)ResourceManager.Images.GetResource(ImageHandle);
    *Image = image();
    Image->Init(ImageData, CreateInfo);
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

vertexBufferHandle context::CreateVertexBuffer(const vertexBufferCreateInfo &CreateInfo)
{
    GET_CONTEXT(GLData, this);

    vertexBufferHandle Handle = this->ResourceManager.VertexBuffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        assert(false);
        return Handle;
    }
    

    vertexBuffer *VertexBuffer = (vertexBuffer*)this->ResourceManager.VertexBuffers.GetResource(Handle);
    VertexBuffer->NumVertexStreams = CreateInfo.NumVertexStreams;
    memcpy(&VertexBuffer->VertexStreams[0], &CreateInfo.VertexStreams[0], commonConstants::MaxVertexStreams * sizeof(vertexStreamData));
    
    VertexBuffer->ApiData = std::make_shared<glVertexBuffer>();
    std::shared_ptr<glVertexBuffer> GLVertexBuffer = std::static_pointer_cast<glVertexBuffer>(VertexBuffer->ApiData);

    //Create the vao
    glGenVertexArrays(1, &GLVertexBuffer->VAO);

    glBindVertexArray(GLVertexBuffer->VAO);
    //Create the vbos
    for(sz i=0; i<VertexBuffer->NumVertexStreams; i++)
    {
        GLVertexBuffer->VertexBuffers[i] = context::Get()->ResourceManager.Buffers.ObtainResource();
        buffer *Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(GLVertexBuffer->VertexBuffers[i]);
        Buffer->ApiData = std::make_shared<glBuffer>();

        Buffer->Init(VertexBuffer->VertexStreams[i].Size, bufferUsage::VertexBuffer, memoryUsage::GpuOnly);
        GLData->CheckErrors();
        Buffer->CopyData((u8*)VertexBuffer->VertexStreams[i].Data, VertexBuffer->VertexStreams[i].Size, 0);
        GLData->CheckErrors();
        std::shared_ptr<glBuffer> GLBuffer = std::static_pointer_cast<glBuffer>(Buffer->ApiData);
        
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
    GET_CONTEXT(GLData, this);
    GLData->CheckErrors();
}


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

    //Depth stencil state
    GLPipeline->DepthStencil.FrontStencilOp = StencilStateToNative(PipelineCreation.DepthStencil.Front);
    GLPipeline->DepthStencil.BackStencilOp = StencilStateToNative(PipelineCreation.DepthStencil.Back);
    GLPipeline->DepthStencil.DepthEnable = PipelineCreation.DepthStencil.DepthEnable;
    GLPipeline->DepthStencil.DepthWrite = PipelineCreation.DepthStencil.DepthWriteEnable;
    GLPipeline->DepthStencil.StencilEnable = PipelineCreation.DepthStencil.StencilEnable;
    GLPipeline->DepthStencil.DepthComparison = CompareOpToNative(PipelineCreation.DepthStencil.DepthComparison);

    //Blend state
    GLPipeline->Blend.Enabled=false;
    if(PipelineCreation.BlendState.ActiveStates>0)
    {
        GLPipeline->Blend.Enabled=true;
        GLPipeline->Blend.BlendSourceColor = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[0].SourceColor);
        GLPipeline->Blend.BlendDestColor = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[0].DestinationColor);
        GLPipeline->Blend.BlendSourceAlpha = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[0].SourceAlpha);
        GLPipeline->Blend.BlendDestAlpha = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[0].DestinationAlpha);
        GLPipeline->Blend.ColorOp = BlendOpToNative(PipelineCreation.BlendState.BlendStates[0].ColorOp);
        GLPipeline->Blend.AlphaOp = BlendOpToNative(PipelineCreation.BlendState.BlendStates[0].AlphaOp);
        GLPipeline->Blend.Separate = PipelineCreation.BlendState.BlendStates[0].SeparateBlend;
    }

    //Rasterizer state
    GLPipeline->Rasterizer.CullMode = CullModeToNative(PipelineCreation.Rasterization.CullMode);
    GLPipeline->Rasterizer.FillMode = FillModeToNative(PipelineCreation.Rasterization.Fill);
    GLPipeline->Rasterizer.FrontFace = FrontFaceToNative(PipelineCreation.Rasterization.FrontFace);


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
    pipeline *Pipeline = (pipeline *) ResourceManager.Pipelines.GetResource(PipelineHandle);
    std::shared_ptr<glPipeline> VkPipelineData = std::static_pointer_cast<glPipeline>(Pipeline->ApiData);


    if(VkPipelineData->ShaderProgram->VertexShader != nullptr) glDeleteShader(VkPipelineData->ShaderProgram->VertexShader->ShaderObject);
    if(VkPipelineData->ShaderProgram->GeometryShader != nullptr) glDeleteShader(VkPipelineData->ShaderProgram->GeometryShader->ShaderObject);
    if(VkPipelineData->ShaderProgram->FragmentShader != nullptr) glDeleteShader(VkPipelineData->ShaderProgram->FragmentShader->ShaderObject);

    glDeleteProgram(VkPipelineData->ShaderProgram->ProgramShaderObject);

    ResourceManager.Pipelines.ReleaseResource(PipelineHandle);


}

void context::DestroyBuffer(bufferHandle BufferHandle)
{
    buffer *Buffer = (buffer*)ResourceManager.Buffers.GetResource(BufferHandle);
    std::shared_ptr<glBuffer> GLBuffer = std::static_pointer_cast<glBuffer>(Buffer->ApiData);
    glDeleteBuffers(1, &GLBuffer->Handle);
    ResourceManager.Buffers.ReleaseResource(BufferHandle);
}

void context::DestroyVertexBuffer(vertexBufferHandle VertexBufferHandle)
{
    vertexBuffer *VertexBuffer = (vertexBuffer *) ResourceManager.VertexBuffers.GetResource(VertexBufferHandle);
    for (sz i = 0; i < VertexBuffer->NumVertexStreams; i++)
    {
        DestroyBuffer(VertexBuffer->VertexStreams[i].Buffer);
    }
    
    std::shared_ptr<glVertexBuffer> GLVertexBuffer = std::static_pointer_cast<glVertexBuffer>(VertexBuffer->ApiData);
    glDeleteVertexArrays(1, &GLVertexBuffer->VAO);

    ResourceManager.VertexBuffers.ReleaseResource(VertexBufferHandle);
}

void context::DestroyImage(imageHandle ImageHandle)
{
    image *Image = (image*)ResourceManager.Images.GetResource(ImageHandle);
    std::shared_ptr<glImage> GLImage = std::static_pointer_cast<glImage>(Image->ApiData);
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
    framebuffer *Framebuffer = (framebuffer*)ResourceManager.Framebuffers.GetResource(FramebufferHandle);
    std::shared_ptr<glFramebufferData> GLFramebuffer = std::static_pointer_cast<glFramebufferData>(Framebuffer->ApiData);
    
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