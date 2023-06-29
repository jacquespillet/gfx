#include "../Include/CommandBuffer.h"
#include "../Include/GfxContext.h"
#include "../Include/Buffer.h"
#include "../Include/Framebuffer.h"
#include "GLCommandBuffer.h"
#include "GLBuffer.h"
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

void ExecuteClearColor(const command &Command)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glClearColor(Command.Clear.ClearColor[0], Command.Clear.ClearColor[1], Command.Clear.ClearColor[2], Command.Clear.ClearColor[3]);
}

void ExecuteClearDepthStencil(const command &Command)
{
    glClear(GL_DEPTH_BUFFER_BIT);
    glClearDepth(Command.Clear.ClearDepth);
    glClear(GL_STENCIL_BUFFER_BIT);
    glClearStencil(Command.Clear.ClearStencil);
}

void ExecuteSetViewport(const command &Command)
{
    glViewport(Command.Viewport.StartX, Command.Viewport.StartY, Command.Viewport.Width, Command.Viewport.Height);
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
}

void ExecuteBindVertexBuffer(const command &Command)
{
    bufferHandle VertexBufferHandle = Command.BindVertexBuffer.VertexBufferHandle;
    buffer *VertexBuffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(VertexBufferHandle);
    std::shared_ptr<glBuffer> GLVertexBuffer = std::static_pointer_cast<glBuffer>(VertexBuffer->ApiData);

    glBindVertexArray(GLVertexBuffer->VAO);
    glBindBuffer(GLVertexBuffer->Target, GLVertexBuffer->Handle);
}

void ExecuteDrawTriangles(const command &Command)
{
    glDrawArrays(GL_TRIANGLES, Command.DrawTriangles.Start, Command.DrawTriangles.Count);
}

void ExecuteBindGraphicsPipeline(const command &Command)
{
    glEnable(GL_SCISSOR_TEST);

    pipeline *Pipeline = (pipeline*)context::Get()->ResourceManager.Pipelines.GetResource(Command.BindGraphicsPipeline.Pipeline);
    std::shared_ptr<glPipeline> GLPipeline = std::static_pointer_cast<glPipeline>(Pipeline->ApiData);
    
    glUseProgram(GLPipeline->ShaderProgram->ProgramShaderObject);
}

void commandBuffer::Begin()
{
    GET_GL_COMMANDS
    assert(!GLCommandBuffer->IsRecording);
    
    GLCommandBuffer->Commands.clear();
    GLCommandBuffer->IsRecording=true;
}

void commandBuffer::BeginPass(renderPassHandle RenderPass, framebufferHandle Framebuffer)
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::BeginPass;
    Command.BeginPass.FramebufferHandle = Framebuffer;
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
void commandBuffer::BindVertexBuffer(bufferHandle Buffer)
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::BindVertexBuffer;
    Command.BindVertexBuffer.VertexBufferHandle = Buffer;
    Command.CommandFunction = (commandFunction)&ExecuteBindVertexBuffer;
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
void commandBuffer::ClearColor(f32 R, f32 G,f32 B,f32 A)
{
    GET_GL_COMMANDS

    command Command;
    Command.Type = commandType::ClearColor;
    Command.Clear.ClearColor[0] = R;
    Command.Clear.ClearColor[1] = G;
    Command.Clear.ClearColor[2] = B;
    Command.Clear.ClearColor[3] = A;
    Command.CommandFunction = (commandFunction)&ExecuteClearColor;

    GLCommandBuffer->Commands.push_back(Command);
}
void commandBuffer::ClearDepthStencil(f32 Depth, f32 Stencil)
{
    GET_GL_COMMANDS

    command Command;
    Command.Type = commandType::ClearColor;
    Command.Clear.ClearDepth = Depth;
    Command.Clear.ClearStencil = Stencil;
    Command.CommandFunction = (commandFunction)&ExecuteClearDepthStencil;

    GLCommandBuffer->Commands.push_back(Command);    
}
void commandBuffer::DrawTriangles(uint32_t Start, uint32_t Count)
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::DrawTriangles;
    Command.DrawTriangles.Start = Start;
    Command.DrawTriangles.Count = Count;
    Command.CommandFunction = (commandFunction)&ExecuteDrawTriangles;
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

}