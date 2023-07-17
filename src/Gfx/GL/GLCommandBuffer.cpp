#include "../Include/CommandBuffer.h"
#include "../Include/GfxContext.h"
#include "../Include/Buffer.h"
#include "../Include/Image.h"
#include "../Include/Framebuffer.h"
#include "GLCommandBuffer.h"
#include "GLBuffer.h"
#include "GLImage.h"
#include "GLPipeline.h"
#include "GLShader.h"
#include "GLFramebuffer.h"
#include <GL/glew.h>

#define GET_GL_COMMANDS std::shared_ptr<glCommandBuffer> GLCommandBuffer = std::static_pointer_cast<glCommandBuffer>(this->ApiData);
namespace gfx
{

void commandBuffer::Initialize()
{
    this->ApiData = std::make_shared<glCommandBuffer>();
}

void ExecuteEndPass(const command &Command)
{
    glBindFramebuffer(GL_FRAMEBUFFER,0);
}


void ExecuteSetViewport(const command &Command)
{
    glViewport((GLint)Command.Viewport.StartX, (GLint)Command.Viewport.StartY, (GLsizei)Command.Viewport.Width, (GLsizei)Command.Viewport.Height);
}

void ExecuteSetScissor(const command &Command)
{
    glScissor(Command.Scissor.StartX, Command.Scissor.StartY, Command.Scissor.Width, Command.Scissor.Height);
}

void ExecuteBeginPass(const command &Command)
{
    framebuffer *Framebuffer = (framebuffer*)context::Get()->ResourceManager.Framebuffers.GetResource(Command.BeginPass.FramebufferHandle);
    std::shared_ptr<glFramebufferData> GLFramebuffer = std::static_pointer_cast<glFramebufferData>(Framebuffer->ApiData);

    glBindFramebuffer(GL_FRAMEBUFFER, GLFramebuffer->Handle);

    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(Command.BeginPass.ClearColor[0], Command.BeginPass.ClearColor[1], Command.BeginPass.ClearColor[2], Command.BeginPass.ClearColor[3]);
    glClear(GL_DEPTH_BUFFER_BIT);
    glClearDepth(Command.BeginPass.ClearDepth);
    glClear(GL_STENCIL_BUFFER_BIT);
    glClearStencil(Command.BeginPass.ClearStencil);
}
void ExecuteBindUniformBuffer(const command &Command)
{
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, Command.BindUniformBuffer.Binding, Command.BindUniformBuffer.Buffer);
}

void ExecuteBindUniformImage(const command &Command)
{
    glBindTextureUnit(Command.BindUniformImage.Binding, Command.BindUniformImage.Image);
}

void ExecuteBindVertexBuffer(const command &Command)
{
    bufferHandle VertexBufferHandle = Command.BindVertexBuffer.VertexBufferHandle;
    vertexBuffer *VertexBuffer = (vertexBuffer*)context::Get()->ResourceManager.VertexBuffers.GetResource(VertexBufferHandle);
    std::shared_ptr<glVertexBuffer> GLVertexBuffer = std::static_pointer_cast<glVertexBuffer>(VertexBuffer->ApiData);

    glBindVertexArray(GLVertexBuffer->VAO);
}

void ExecuteBindIndexBuffer(const command &Command)
{
    bufferHandle IndexBufferHandle = Command.BindIndexBuffer.IndexBufferHandle;
    buffer *IndexBuffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(IndexBufferHandle);
    std::shared_ptr<glBuffer> GLIndexBuffer = std::static_pointer_cast<glBuffer>(IndexBuffer->ApiData);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GLIndexBuffer->Handle);
}

void ExecuteDrawIndexed(const command &Command)
{
    //TODO: Store the type of the indices and reset it right after.
    if(Command.DrawIndexed.InstanceCount>1)
    {
        glDrawElementsInstanced(GL_TRIANGLES, Command.DrawIndexed.Count, GL_UNSIGNED_INT, nullptr, Command.DrawIndexed.InstanceCount);
    }
    else
    {
        glDrawElements(GL_TRIANGLES, Command.DrawIndexed.Count, GL_UNSIGNED_INT, nullptr);
    }
}

void ExecuteDrawTriangles(const command &Command)
{
    if(Command.DrawIndexed.InstanceCount>1)
    {
        glDrawArraysInstanced(GL_TRIANGLES, Command.DrawTriangles.Start, Command.DrawTriangles.Count, Command.DrawTriangles.InstanceCount);
    }
    else
    {
        glDrawArrays(GL_TRIANGLES, Command.DrawTriangles.Start, Command.DrawTriangles.Count);
    }
}


void ExecuteBindGraphicsPipeline(const command &Command)
{
    glEnable(GL_SCISSOR_TEST);

    pipeline *Pipeline = (pipeline*)context::Get()->ResourceManager.Pipelines.GetResource(Command.BindGraphicsPipeline.Pipeline);
    std::shared_ptr<glPipeline> GLPipeline = std::static_pointer_cast<glPipeline>(Pipeline->ApiData);
    
    glUseProgram(GLPipeline->ShaderProgram->ProgramShaderObject);

    //Bind depth stencil
    GLPipeline->DepthStencil.DepthEnable ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
    GLPipeline->DepthStencil.DepthWrite ? glDepthMask(GL_TRUE) : glDepthMask(GL_FALSE);
    GLPipeline->DepthStencil.StencilEnable ? glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST);
    glDepthFunc(GLPipeline->DepthStencil.DepthComparison);
    if(GLPipeline->DepthStencil.StencilEnable)
    {
        glStencilFuncSeparate(GL_FRONT, GLPipeline->DepthStencil.FrontStencilOp.Operation, 0, 0xFF);
        glStencilOpSeparate(GL_FRONT, GLPipeline->DepthStencil.FrontStencilOp.Fail, GLPipeline->DepthStencil.FrontStencilOp.DepthFail, GLPipeline->DepthStencil.FrontStencilOp.Pass);
        glStencilFuncSeparate(GL_BACK, GLPipeline->DepthStencil.BackStencilOp.Operation, 0, 0xFF);
        glStencilOpSeparate(GL_BACK, GLPipeline->DepthStencil.BackStencilOp.Fail, GLPipeline->DepthStencil.BackStencilOp.DepthFail, GLPipeline->DepthStencil.BackStencilOp.Pass);
    }
    
    //Bind blend
    if(GLPipeline->Blend.Enabled)
    {
        glEnable(GL_BLEND);
        if(GLPipeline->Blend.Separate)
        {
            glBlendFunc(GLPipeline->Blend.BlendSourceColor, GLPipeline->Blend.BlendDestColor);
            glBlendEquation(GLPipeline->Blend.ColorOp);
        }
        else
        {
            glBlendFuncSeparate(GLPipeline->Blend.BlendSourceColor, GLPipeline->Blend.BlendDestColor, GLPipeline->Blend.BlendSourceAlpha, GLPipeline->Blend.BlendDestAlpha);
            glBlendEquationSeparate(GLPipeline->Blend.ColorOp, GLPipeline->Blend.AlphaOp);
        }
    }
    else
    {
        glDisable(GL_BLEND);
    }

    //bind raster   
    if(GLPipeline->Rasterizer.CullMode != 0)
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GLPipeline->Rasterizer.CullMode);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    glFrontFace(GLPipeline->Rasterizer.FrontFace);
    glPolygonMode(GL_FRONT_AND_BACK, GLPipeline->Rasterizer.FillMode);


}

void commandBuffer::Begin()
{
    GET_GL_COMMANDS
    assert(!GLCommandBuffer->IsRecording);
    
    GLCommandBuffer->Commands.clear();
    GLCommandBuffer->IsRecording=true;
}

void commandBuffer::BeginPass(framebufferHandle Framebuffer, clearColorValues ClearColor, clearDepthStencilValues DepthStencil)
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::BeginPass;
    Command.BeginPass.FramebufferHandle = Framebuffer;
    Command.BeginPass.ClearColor[0] = ClearColor.R;
    Command.BeginPass.ClearColor[1] = ClearColor.G;
    Command.BeginPass.ClearColor[2] = ClearColor.B;
    Command.BeginPass.ClearColor[3] = ClearColor.A;
    Command.BeginPass.ClearDepth = DepthStencil.Depth;
    Command.BeginPass.ClearStencil = DepthStencil.Stencil;

    Command.CommandFunction = (commandFunction)&ExecuteBeginPass;
    GLCommandBuffer->Commands.push_back(Command);
}

void commandBuffer::EndPass()
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::EndPass;
    Command.CommandFunction = (commandFunction)&ExecuteEndPass;
    GLCommandBuffer->Commands.push_back(Command);
}

void commandBuffer::BindGraphicsPipeline(pipelineHandle Pipeline)
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::BindPipeline;
    Command.BindGraphicsPipeline.Pipeline = Pipeline;
    Command.CommandFunction = (commandFunction)&ExecuteBindGraphicsPipeline;
    GLCommandBuffer->Commands.push_back(Command);
}
void commandBuffer::BindVertexBuffer(vertexBufferHandle Buffer)
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::BindVertexBuffer;
    Command.BindVertexBuffer.VertexBufferHandle = Buffer;
    Command.CommandFunction = (commandFunction)&ExecuteBindVertexBuffer;
    GLCommandBuffer->Commands.push_back(Command);
}

//TODO: Use offset and index type
void commandBuffer::BindIndexBuffer(bufferHandle Buffer, u32 Offset, indexType IndexType)
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::BindIndexBuffer;
    Command.BindIndexBuffer.IndexBufferHandle = Buffer;
    Command.CommandFunction = (commandFunction)&ExecuteBindIndexBuffer;
    GLCommandBuffer->Commands.push_back(Command);
}

void commandBuffer::SetViewport(f32 X, f32 Y, f32 Width, f32 Height)
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::SetViewport;
    Command.Viewport.StartX = X;
    Command.Viewport.StartY = Y;
    Command.Viewport.Width = Width;
    Command.Viewport.Height = Height;

    Command.CommandFunction = (commandFunction)&ExecuteSetViewport;
    GLCommandBuffer->Commands.push_back(Command);
}
void commandBuffer::SetScissor(s32 X, s32 Y, u32 Width, u32 Height)
{

    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::SetScissor;
    Command.Scissor.StartX = X;
    Command.Scissor.StartY = Y;
    Command.Scissor.Width = Width;
    Command.Scissor.Height = Height;

    Command.CommandFunction = (commandFunction)&ExecuteSetScissor;
    GLCommandBuffer->Commands.push_back(Command);
}

void commandBuffer::DrawArrays(u32 Start, u32 Count, u32 InstanceCount)
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::DrawTriangles;
    Command.DrawTriangles.Start = Start;
    Command.DrawTriangles.Count = Count;
    Command.DrawTriangles.InstanceCount = InstanceCount;
    Command.CommandFunction = (commandFunction)&ExecuteDrawTriangles;
    GLCommandBuffer->Commands.push_back(Command);
}

//TODO: Use Start
void commandBuffer::DrawIndexed(u32 Start, u32 Count, u32 InstanceCount)
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::DrawIndexed;
    Command.DrawIndexed.Count = Count;
    Command.DrawIndexed.InstanceCount = InstanceCount;
    Command.CommandFunction = (commandFunction)&ExecuteDrawIndexed;
    GLCommandBuffer->Commands.push_back(Command);
}
    
void commandBuffer::End()
{
    GET_GL_COMMANDS
    GLCommandBuffer->IsRecording=false;
    for (size_t i = 0; i < GLCommandBuffer->Commands.size(); i++)
    {
        GLCommandBuffer->Commands[i].CommandFunction(GLCommandBuffer->Commands[i]);
    }
    GLCommandBuffer->Commands.resize(0);
}

void commandBuffer::BindUniformGroup(std::shared_ptr<uniformGroup> Group, u32 Binding)
{
    GET_GL_COMMANDS

    
    for(sz i=0; i< Group->Uniforms.size(); i++)
    {
        if(Group->Uniforms[i].Type == uniformType::Buffer)
        {
            buffer* BufferData = (buffer*)(Group->Uniforms[i].Resource);
            std::shared_ptr<glBuffer> GLBuffer = std::static_pointer_cast<glBuffer>(BufferData->ApiData);

            command Command;
            Command.Type = commandType::DrawTriangles;
            Command.BindUniformBuffer.Binding = Group->Uniforms[i].Binding;
            Command.BindUniformBuffer.Buffer = GLBuffer->Handle;
            Command.CommandFunction = (commandFunction)&ExecuteBindUniformBuffer;
            GLCommandBuffer->Commands.push_back(Command);
        }
        if(Group->Uniforms[i].Type == uniformType::Texture2d)
        {
            image* ImageData = (image*)(Group->Uniforms[i].Resource);
            std::shared_ptr<glImage> GLImage = std::static_pointer_cast<glImage>(ImageData->ApiData);

            command Command;
            Command.Type = commandType::DrawTriangles;
            Command.BindUniformImage.Binding = Group->Uniforms[i].Binding;
            Command.BindUniformImage.Image = GLImage->Handle;
            Command.CommandFunction = (commandFunction)&ExecuteBindUniformImage;
            GLCommandBuffer->Commands.push_back(Command);
        }
    }
}

}