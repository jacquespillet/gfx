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

#include "imgui_impl_dx11.h"


#define GET_GL_COMMANDS std::shared_ptr<d3d11CommandBuffer> GLCommandBuffer = std::static_pointer_cast<d3d11CommandBuffer>(this->ApiData);
namespace gfx
{

void commandBuffer::Initialize()
{
    this->ApiData = std::make_shared<d3d11CommandBuffer>();
}

void ExecuteEndPass(const command &Command, d3d11CommandBuffer &CommandBuffer)
{
    GET_CONTEXT(D11Data, context::Get());
    ID3D11ShaderResourceView* nullSRV[] = { nullptr };
    for (sz i = 0; i < 16; i++)
    {
        D11Data->DeviceContext->VSSetShaderResources(i, 1, nullSRV);
        D11Data->DeviceContext->PSSetShaderResources(i, 1, nullSRV);
    }
}


void ExecuteSetViewport(const command &Command, d3d11CommandBuffer &CommandBuffer)
{
    GET_CONTEXT(D11Data, context::Get());

    D3D11_VIEWPORT Viewport = { Command.Viewport.StartX, Command.Viewport.StartY, Command.Viewport.Width, Command.Viewport.Height, 0.0f, 1.0f };
    D11Data->DeviceContext->RSSetViewports(1, &Viewport);
}

void ExecuteSetScissor(const command &Command, d3d11CommandBuffer &CommandBuffer)
{
    GET_CONTEXT(D11Data, context::Get());

    D3D11_RECT Scissor = { Command.Scissor.StartX, Command.Scissor.StartY, Command.Scissor.Width, Command.Scissor.Height};
    D11Data->DeviceContext->RSSetScissorRects(1, &Scissor);
}

void ExecuteBeginPass(const command &Command, d3d11CommandBuffer &CommandBuffer)
{
    GET_CONTEXT(D11Data, context::Get());
    framebuffer *Framebuffer = context::Get()->GetFramebuffer(Command.BeginPass.FramebufferHandle);
    GET_API_DATA(D11Framebuffer, d3d11FramebufferData, Framebuffer)
    
    //TODO
    FLOAT BackgroundColor[4] = { Command.BeginPass.ClearColor[0],Command.BeginPass.ClearColor[1],Command.BeginPass.ClearColor[2],Command.BeginPass.ClearColor[3] };
    ID3D11RenderTargetView *RTV = {D11Framebuffer->ColorViews[0].Get()};
    D11Data->DeviceContext->OMSetRenderTargets(D11Framebuffer->RenderTargetCount, D11Framebuffer->ColorViews[0].GetAddressOf(), D11Framebuffer->DepthBufferView.Get());
    for (sz i = 0; i < D11Framebuffer->RenderTargetCount; i++)
    {
        D11Data->DeviceContext->ClearRenderTargetView(D11Framebuffer->ColorViews[i].Get(), BackgroundColor);
    }
    D11Data->DeviceContext->ClearDepthStencilView(D11Framebuffer->DepthBufferView.Get(), D3D11_CLEAR_DEPTH, Command.BeginPass.ClearDepth, Command.BeginPass.ClearStencil);
}

void ExecuteBindUniformBuffer(const command &Command, d3d11CommandBuffer &CommandBuffer)
{
    GET_CONTEXT(D11Data, context::Get());
    pipeline *BoundPipeline = context::Get()->GetPipeline(CommandBuffer.BoundPipeline);
    if(BoundPipeline->GraphicsPipeline)
    {
        D11Data->DeviceContext->VSSetConstantBuffers(Command.BindUniformBuffer.Binding, 1, &Command.BindUniformBuffer.Buffer);
        D11Data->DeviceContext->PSSetConstantBuffers(Command.BindUniformBuffer.Binding, 1, &Command.BindUniformBuffer.Buffer);
    }
    else
    {
        D11Data->DeviceContext->CSSetConstantBuffers(Command.BindUniformBuffer.Binding, 1, &Command.BindUniformBuffer.Buffer);
    }
}

void ExecuteBindStorageBuffer(const command &Command, d3d11CommandBuffer &CommandBuffer)
{
    GET_CONTEXT(D11Data, context::Get());
    pipeline *BoundPipeline = context::Get()->GetPipeline(CommandBuffer.BoundPipeline);
    if(BoundPipeline->GraphicsPipeline)
    {
        D11Data->DeviceContext->VSSetShaderResources(Command.BindStorageBuffer.Binding, 1, &Command.BindStorageBuffer.SRV);
        D11Data->DeviceContext->PSSetShaderResources(Command.BindStorageBuffer.Binding, 1, &Command.BindStorageBuffer.SRV);
    }
    else
    {
        D11Data->DeviceContext->CSSetUnorderedAccessViews(Command.BindStorageBuffer.Binding, 1, &Command.BindStorageBuffer.UAV, nullptr);
    }
}

void ExecuteBindUniformImage(const command &Command, d3d11CommandBuffer &CommandBuffer)
{
    GET_CONTEXT(D11Data, context::Get());
    pipeline *BoundPipeline = context::Get()->GetPipeline(CommandBuffer.BoundPipeline);
    if(BoundPipeline->GraphicsPipeline)
    {
        D11Data->DeviceContext->PSSetShaderResources(Command.BindUniformImage.Binding, 1, &Command.BindUniformImage.Image);
    }
    else
    {
        D11Data->DeviceContext->CSSetShaderResources(Command.BindUniformImage.Binding, 1, &Command.BindUniformImage.Image);
    }
}

void ExecuteBindVertexBuffer(const command &Command, d3d11CommandBuffer &CommandBuffer)
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
        D11Data->DeviceContext->IASetVertexBuffers(VertexBuffer->VertexStreams[i].StreamIndex, 1, D11Buffer->Handle.GetAddressOf(), &Stride, &Offset);
    }
    
}

void ExecuteBindIndexBuffer(const command &Command, d3d11CommandBuffer &CommandBuffer)
{
    GET_CONTEXT(D11Data, context::Get());
    bufferHandle BufferHandle = Command.BindIndexBuffer.IndexBufferHandle;
    buffer *IndexBuffer = context::Get()->GetBuffer(BufferHandle);
    GET_API_DATA(D11Buffer, d3d11Buffer, IndexBuffer);
    
    D11Data->DeviceContext->IASetIndexBuffer(D11Buffer->Handle.Get(), IndexTypeToNative(Command.BindIndexBuffer.IndexType), Command.BindIndexBuffer.Offset);
}

void ExecuteDrawImgui(const command &Command, d3d11CommandBuffer &CommandBuffer)
{
    ImGui::Render();
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ExecuteDrawIndexed(const command &Command, d3d11CommandBuffer &CommandBuffer)
{
    GET_CONTEXT(D11Data, context::Get());
    D11Data->DeviceContext->DrawIndexed(Command.DrawIndexed.Count, Command.DrawIndexed.Offset, 0);
}

void ExecuteDrawTriangles(const command &Command, d3d11CommandBuffer &CommandBuffer)
{
    GET_CONTEXT(D11Data, context::Get());
    D11Data->DeviceContext->DrawInstanced(Command.DrawTriangles.Count, Command.DrawTriangles.InstanceCount, Command.DrawTriangles.Start, 0);
}


void ExecuteBindGraphicsPipeline(const command &Command, d3d11CommandBuffer &CommandBuffer)
{
    GET_CONTEXT(D11Data, context::Get());
    pipeline *Pipeline = context::Get()->GetPipeline(Command.BindGraphicsPipeline.Pipeline);
    GET_API_DATA(D11Pipeline, d3d11Pipeline, Pipeline);
    
    D11Data->DeviceContext->RSSetState(D11Pipeline->RasterizerState.Get());
    D11Data->DeviceContext->PSSetSamplers(0, 1, D11Pipeline->SamplerState.GetAddressOf());
    D11Data->DeviceContext->OMSetDepthStencilState(D11Pipeline->DepthStencilState.Get(), 0);
    D11Data->DeviceContext->IASetInputLayout(D11Pipeline->InputLayout.Get());
    D11Data->DeviceContext->VSSetShader(D11Pipeline->VertexShader.Get(), nullptr, 0);
    D11Data->DeviceContext->PSSetShader(D11Pipeline->PixelShader.Get(), nullptr, 0);
    D11Data->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    UINT SampleMask = 0xffffffff;
    D11Data->DeviceContext->OMSetBlendState(D11Pipeline->BlendState.Get(), nullptr, SampleMask);
  
    CommandBuffer.BoundPipeline = Command.BindGraphicsPipeline.Pipeline;
}

void ExecuteBindComputePipeline(const command &Command, d3d11CommandBuffer &CommandBuffer)
{
    GET_CONTEXT(D11Data, context::Get());
    pipeline *Pipeline = context::Get()->GetPipeline(Command.BindGraphicsPipeline.Pipeline);
    GET_API_DATA(D11Pipeline, d3d11Pipeline, Pipeline);
    D11Data->DeviceContext->CSSetShader(D11Pipeline->ComputeShader.Get(), nullptr, 0);

    CommandBuffer.BoundPipeline = Command.BindGraphicsPipeline.Pipeline;
}

void ExecuteDispatchCompute(const command &Command, d3d11CommandBuffer &CommandBuffer)
{
    GET_CONTEXT(D11Data, context::Get());
    D11Data->DeviceContext->Dispatch(Command.DispatchCompute.NumGroupX, Command.DispatchCompute.NumGroupY, Command.DispatchCompute.NumGroupZ);

    for (sz i = 0; i < 16; i++)
    {
        ID3D11UnorderedAccessView* nullUAV[] = { nullptr };
        D11Data->DeviceContext->CSSetUnorderedAccessViews(i, 1, nullUAV, nullptr);
    }
    D11Data->DeviceContext->CSSetShader(nullptr, nullptr, 0);
    
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
        GLCommandBuffer->Commands[i].CommandFunction(GLCommandBuffer->Commands[i], *GLCommandBuffer);
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
            Command.BindUniformBuffer.Buffer = D11Buffer->Handle.Get();
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
            Command.BindStorageBuffer.SRV = D11Buffer->StructuredHandle.Get();
            Command.BindStorageBuffer.UAV = D11Buffer->UAVHandle.Get();
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
            Command.BindUniformImage.Image = D11Image->View.Get();
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
            Command.BindUniformImage.Image = D11Framebuffer->SRVViews[Group->Uniforms[i].ResourceIndex].Get();
            Command.CommandFunction = (commandFunction)&ExecuteBindUniformImage;
            GLCommandBuffer->Commands.push_back(Command);
        }
    }
}

}