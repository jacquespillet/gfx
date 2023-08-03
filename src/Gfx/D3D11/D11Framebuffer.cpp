#include "../Include/Context.h"
#include "../Include/Framebuffer.h"
#include "D11Framebuffer.h"
#include "D11Context.h"
#include "D11Common.h"
#include "D11Mapping.h"

namespace gfx
{

void d3d11FramebufferData::AddRenderTarget(format Format, const f32 *ClearValues)
{
    GET_CONTEXT(D11Data, context::Get());

    D3D11_TEXTURE2D_DESC colorBufferDesc;
    ZeroMemory(&colorBufferDesc, sizeof(colorBufferDesc));
    colorBufferDesc.Width = this->Framebuffer->Width;
    colorBufferDesc.Height = this->Framebuffer->Height;
    colorBufferDesc.MipLevels = 1;
    colorBufferDesc.ArraySize = 1;
    colorBufferDesc.Format = FormatToNative(Format); // Choose the appropriate format
    colorBufferDesc.SampleDesc.Count = 1;
    colorBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    colorBufferDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    D11Data->Device->CreateTexture2D(&colorBufferDesc, nullptr, this->ColorHandles[RenderTargetCount].GetAddressOf());
    D11Data->Device->CreateRenderTargetView(this->ColorHandles[RenderTargetCount].Get(), nullptr, this->ColorViews[RenderTargetCount].GetAddressOf());
    
    
    D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc = {};
    ShaderResourceViewDesc.Format = FormatToNative(Format);
    ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    ShaderResourceViewDesc.Texture2D.MipLevels = 1;
    ShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
    D11Data->Device->CreateShaderResourceView(this->ColorHandles[RenderTargetCount].Get(), &ShaderResourceViewDesc, this->SRVViews[RenderTargetCount].GetAddressOf());

    RenderTargetCount++;  
}
void d3d11FramebufferData::CreateDepthBuffer(u32 Width, u32 Height, format Format)
{
    GET_CONTEXT(D11Data, context::Get());

    D3D11_TEXTURE2D_DESC depthBufferDesc;
    ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));
    depthBufferDesc.Width = Framebuffer->Width;               // Width of the texture
    depthBufferDesc.Height = Framebuffer->Height;             // Height of the texture
    depthBufferDesc.MipLevels = 1;
    depthBufferDesc.ArraySize = 1;
    depthBufferDesc.Format = FormatToNative(Format); // Choose the appropriate format
    depthBufferDesc.SampleDesc.Count = 1;
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    D11Data->Device->CreateTexture2D(&depthBufferDesc, nullptr, DepthBuffer.GetAddressOf());
    D11Data->Device->CreateDepthStencilView(DepthBuffer.Get(), nullptr, DepthBufferView.GetAddressOf());
}
void d3d11FramebufferData::CreateDescriptors()
{

}    
}