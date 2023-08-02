#pragma once

namespace gfx
{
struct d3d11FramebufferData
{
    ID3D11Texture2D* Handle;
    ID3D11RenderTargetView* View;
    ID3D11Texture2D* DepthBuffer;
    ID3D11DepthStencilView* DepthBufferView;
};
}