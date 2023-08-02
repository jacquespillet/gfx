#include "../Include/CommandBuffer.h"
#include "../Include/Context.h"
#include "../Include/Buffer.h"
#include "../Include/Image.h"
#include "../Include/Framebuffer.h"
#include "../Include/Types.h"
#include "D11CommandBuffer.h"
#include "D11Buffer.h"
#include "D11Image.h"
#include "D11Pipeline.h"
#include "D11Common.h"
#include "D11Framebuffer.h"
#include "D11Context.h"
#include "D11Mapping.h"

#include <GL/glew.h>

#define GET_GL_COMMANDS std::shared_ptr<d3d11CommandBuffer> GLCommandBuffer = std::static_pointer_cast<d3d11CommandBuffer>(this->ApiData);
namespace gfx
{

void commandBuffer::Initialize()
{
    this->ApiData = std::make_shared<d3d11CommandBuffer>();
}

void ExecuteEndPass(const command &Command)
{
    
}


void ExecuteSetViewport(const command &Command)
{
    GET_CONTEXT(D11Data, context::Get());

    D3D11_VIEWPORT Viewport = { Command.Viewport.StartX, Command.Viewport.StartY, Command.Viewport.Width, Command.Viewport.Height, 0.0f, 1.0f };
    D11Data->DeviceContext->RSSetViewports(1, &Viewport);
}

void ExecuteSetScissor(const command &Command)
{
    GET_CONTEXT(D11Data, context::Get());

    D3D11_RECT Scissor = { Command.Scissor.StartX, Command.Scissor.StartY, Command.Scissor.Width, Command.Scissor.Height};
    D11Data->DeviceContext->RSSetScissorRects(1, &Scissor);
}

void ExecuteBeginPass(const command &Command)
{
    GET_CONTEXT(D11Data, context::Get());
    framebuffer *Framebuffer = context::Get()->GetFramebuffer(Command.BeginPass.FramebufferHandle);
    GET_API_DATA(D11Framebuffer, d3d11FramebufferData, Framebuffer)
    
    //TODO
    FLOAT BackgroundColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };
    D11Data->DeviceContext->OMSetRenderTargets(1, &D11Framebuffer->View, D11Framebuffer->DepthBufferView);
    D11Data->DeviceContext->ClearRenderTargetView(D11Framebuffer->View, BackgroundColor);
    D11Data->DeviceContext->ClearDepthStencilView(D11Framebuffer->DepthBufferView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void ExecuteBindUniformBuffer(const command &Command)
{
    GET_CONTEXT(D11Data, context::Get());
    D11Data->DeviceContext->PSSetConstantBuffers(Command.BindUniformBuffer.Binding, 1, &Command.BindUniformBuffer.Buffer);
}

void ExecuteBindStorageBuffer(const command &Command)
{
    assert(false);
}

void ExecuteBindUniformImage(const command &Command)
{
    GET_CONTEXT(D11Data, context::Get());
    D11Data->DeviceContext->PSSetShaderResources(Command.BindUniformImage.Binding, 1, &Command.BindUniformImage.Image);
}

void ExecuteBindVertexBuffer(const command &Command)
{
    GET_CONTEXT(D11Data, context::Get());
    vertexBufferHandle VertexBufferHandle = Command.BindVertexBuffer.VertexBufferHandle;
    vertexBuffer *VertexBuffer = context::Get()->GetVertexBuffer(VertexBufferHandle);
    GET_API_DATA(D11VertexBuffer, d3d11VertexBuffer, VertexBuffer);
    
    for (sz i = 0; i < VertexBuffer->NumVertexStreams; i++)
    {
        buffer *VertexStreamBuffer = context::Get()->GetBuffer(D11VertexBuffer->VertexBuffers[i]);
        GET_API_DATA(D11Buffer, d3d11Buffer, VertexStreamBuffer);

        u32 Stride = VertexBuffer->VertexStreams[i].Stride;
        u32 Offset = 0;
        D11Data->DeviceContext->IASetVertexBuffers(VertexBuffer->VertexStreams[i].StreamIndex, 1, &D11Buffer->Handle, &Stride, &Offset);
    }
    
}

void ExecuteBindIndexBuffer(const command &Command)
{
    GET_CONTEXT(D11Data, context::Get());
    bufferHandle BufferHandle = Command.BindIndexBuffer.IndexBufferHandle;
    buffer *IndexBuffer = context::Get()->GetBuffer(BufferHandle);
    GET_API_DATA(D11Buffer, d3d11Buffer, IndexBuffer);
    
    D11Data->DeviceContext->IASetIndexBuffer(D11Buffer->Handle, IndexTypeToNative(Command.BindIndexBuffer.IndexType), Command.BindIndexBuffer.Offset);
}

void ExecuteDrawImgui(const command &Command)
{
    assert(false);
}

void ExecuteDrawIndexed(const command &Command)
{
    GET_CONTEXT(D11Data, context::Get());
    D11Data->DeviceContext->DrawIndexed(Command.DrawIndexed.Count, Command.DrawIndexed.Offset, 0);
}

void ExecuteDrawTriangles(const command &Command)
{
    GET_CONTEXT(D11Data, context::Get());
    D11Data->DeviceContext->DrawInstanced(Command.DrawTriangles.Count, 1, Command.DrawTriangles.Start, 0);
}


void ExecuteBindGraphicsPipeline(const command &Command)
{
    GET_CONTEXT(D11Data, context::Get());
    pipeline *Pipeline = context::Get()->GetPipeline(Command.BindGraphicsPipeline.Pipeline);
    GET_API_DATA(D11Pipeline, d3d11Pipeline, Pipeline);
    
    D11Data->DeviceContext->RSSetState(D11Pipeline->RasterizerState);
    D11Data->DeviceContext->PSSetSamplers(0, 1, &D11Pipeline->SamplerState);
    D11Data->DeviceContext->OMSetDepthStencilState(D11Pipeline->DepthStencilState, 0);
    D11Data->DeviceContext->IASetInputLayout(D11Pipeline->InputLayout);
    D11Data->DeviceContext->VSSetShader(D11Pipeline->VertexShader, nullptr, 0);
    D11Data->DeviceContext->PSSetShader(D11Pipeline->PixelShader, nullptr, 0);
    D11Data->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void ExecuteBindComputePipeline(const command &Command)
{
    assert(false);
}

void ExecuteDispatchCompute(const command &Command)
{
    assert(false);
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

void commandBuffer::BindComputePipeline(pipelineHandle Pipeline)
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::BindPipeline;
    Command.BindGraphicsPipeline.Pipeline = Pipeline;
    Command.CommandFunction = (commandFunction)&ExecuteBindComputePipeline;
    GLCommandBuffer->Commands.push_back(Command);
}


void commandBuffer::Dispatch(u32 NumGroupX, u32 NumGroupY, u32 NumGroupZ)
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::Dispatch;
    Command.DispatchCompute.NumGroupX = NumGroupX;
    Command.DispatchCompute.NumGroupY = NumGroupY;
    Command.DispatchCompute.NumGroupZ = NumGroupZ;
    Command.CommandFunction = (commandFunction)&ExecuteDispatchCompute;
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

void commandBuffer::BindIndexBuffer(bufferHandle Buffer, u32 Offset, indexType IndexType)
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::BindIndexBuffer;
    Command.BindIndexBuffer.IndexBufferHandle = Buffer;
    Command.BindIndexBuffer.Offset = Offset;
    Command.BindIndexBuffer.IndexType = IndexType;
    Command.CommandFunction = (commandFunction)&ExecuteBindIndexBuffer;
    GLCommandBuffer->Commands.push_back(Command);

    GLCommandBuffer->IndexType = IndexType;
    GLCommandBuffer->IndexBufferOffset = Offset;
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

void commandBuffer::DrawIndexed(u32 Start, u32 Count, u32 InstanceCount)
{
    GET_GL_COMMANDS
    command Command;
    Command.Type = commandType::DrawIndexed;
    Command.DrawIndexed.Count = Count;
    Command.DrawIndexed.InstanceCount = InstanceCount;
    Command.DrawIndexed.IndexType = GLCommandBuffer->IndexType; 
    Command.DrawIndexed.Offset = GLCommandBuffer->IndexBufferOffset + Start; 
    Command.CommandFunction = (commandFunction)&ExecuteDrawIndexed;
    GLCommandBuffer->Commands.push_back(Command);

    GLCommandBuffer->IndexType = indexType::Uint16;
    GLCommandBuffer->IndexBufferOffset=0;
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

void d3d11CommandBuffer::DrawImgui()
{
    command Command;
    Command.Type = commandType::DrawImgui;
    Command.CommandFunction = (commandFunction)&ExecuteDrawImgui;
    Commands.push_back(Command);
}

void commandBuffer::BindUniformGroup(std::shared_ptr<uniformGroup> Group, u32 Binding)
{
    GET_GL_COMMANDS

    
    for(sz i=0; i< Group->Uniforms.size(); i++)
    {
        if(Group->Uniforms[i].Type == uniformType::UniformBuffer)
        {
            buffer* BufferData = Group->GetBuffer(i);
            GET_API_DATA(D11Buffer, d3d11Buffer, BufferData);
            command Command;
            Command.Type = commandType::BindUniforms;
            Command.BindUniformBuffer.Binding = Group->Uniforms[i].Binding;
            Command.BindUniformBuffer.Buffer = D11Buffer->Handle;
            Command.CommandFunction = (commandFunction)&ExecuteBindUniformBuffer;
            GLCommandBuffer->Commands.push_back(Command);
        }
        else if(Group->Uniforms[i].Type == uniformType::StorageBuffer)
        {
            buffer* BufferData  = Group->GetBuffer(i);
            GET_API_DATA(D11Buffer, d3d11Buffer, BufferData);

            command Command;
            Command.Type = commandType::BindUniforms;
            Command.BindStorageBuffer.Binding = Group->Uniforms[i].Binding;
            Command.BindStorageBuffer.Buffer = D11Buffer->Handle;
            Command.CommandFunction = (commandFunction)&ExecuteBindStorageBuffer;
            GLCommandBuffer->Commands.push_back(Command);
        }
        else if(Group->Uniforms[i].Type == uniformType::Texture2d)
        {
            image* ImageData  = Group->GetTexture(i);
            GET_API_DATA(D11Image, d3d11Image, ImageData);

            command Command;
            Command.Type = commandType::BindUniforms;
            Command.BindUniformImage.Binding = Group->Uniforms[i].Binding;
            Command.BindUniformImage.Image = D11Image->View;
            Command.CommandFunction = (commandFunction)&ExecuteBindUniformImage;
            GLCommandBuffer->Commands.push_back(Command);
        }
        else if(Group->Uniforms[i].Type == uniformType::FramebufferRenderTarget)
        {

            framebuffer* Framebuffer = Group->GetFramebuffer(i);
            GET_API_DATA(D11Framebuffer, d3d11FramebufferData, Framebuffer)
            

            command Command;
            Command.Type = commandType::BindUniforms;
            Command.BindUniformImage.Binding = Group->Uniforms[i].Binding;
            assert(false);
            // Command.BindUniformImage.Image = D11Framebuffer->ColorTextures[Group->Uniforms[i].ResourceIndex];
            Command.CommandFunction = (commandFunction)&ExecuteBindUniformImage;
            GLCommandBuffer->Commands.push_back(Command);
        }
    }
}

}