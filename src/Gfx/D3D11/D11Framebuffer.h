#pragma once
#include "../Include/Types.h"
#include <d3d11.h>
namespace gfx
{
struct framebuffer;

struct d3d11FramebufferData
{
    framebuffer *Framebuffer;
    ID3D11Texture2D* ColorHandles[commonConstants::MaxImageOutputs];
    ID3D11RenderTargetView* ColorViews[commonConstants::MaxImageOutputs];
    ID3D11ShaderResourceView *SRVViews[commonConstants::MaxImageOutputs];

    ID3D11Texture2D* DepthBuffer;
    ID3D11DepthStencilView* DepthBufferView;

    u32 RenderTargetCount = 0;

    void AddRenderTarget(format Format, const f32 *ClearValues);
    void CreateDepthBuffer(u32 Width, u32 Height, format Format);
    void CreateDescriptors();
};
}