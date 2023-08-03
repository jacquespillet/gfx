#pragma once
#include "../Include/Types.h"
#include <d3d11.h>

#include <wrl.h>
using namespace Microsoft::WRL;

namespace gfx
{
struct framebuffer;

struct d3d11FramebufferData
{
    framebuffer *Framebuffer;

    ComPtr<ID3D11Texture2D> ColorHandles[commonConstants::MaxImageOutputs];
    ComPtr<ID3D11RenderTargetView> ColorViews[commonConstants::MaxImageOutputs];
    ComPtr<ID3D11ShaderResourceView> SRVViews[commonConstants::MaxImageOutputs];
    ComPtr<ID3D11Texture2D> DepthBuffer;
    ComPtr<ID3D11DepthStencilView> DepthBufferView;

    u32 RenderTargetCount = 0;

    void AddRenderTarget(format Format, const f32 *ClearValues);
    void CreateDepthBuffer(u32 Width, u32 Height, format Format);
    void CreateDescriptors();
};
}