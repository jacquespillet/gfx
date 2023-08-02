#pragma once

#include <d3d11_1.h>
#include "../Include/Types.h"

namespace gfx
{
struct d3d11Data
{
    ID3D11Device* BaseDevice;
    ID3D11DeviceContext* BaseDeviceContext;
    ID3D11Device1* Device;
    ID3D11DeviceContext1* DeviceContext;

    
    IDXGIDevice1* DXGIDevice;
    IDXGIAdapter* DXGIAdapter;
    IDXGIFactory2* DXGIFactory;

    framebufferHandle SwapchainFramebuffer;

    std::shared_ptr<commandBuffer> CommandBuffer;
};

}