#pragma once

#include <d3d11_1.h>
#include "../Include/Types.h"
#include "../Include/Buffer.h"

#include <wrl.h>
using namespace Microsoft::WRL;


namespace gfx
{
struct d3d11Data
{
    ComPtr<ID3D11Device> BaseDevice;
    ComPtr<ID3D11DeviceContext> BaseDeviceContext;
    ComPtr<ID3D11Device1> Device;
    ComPtr<ID3D11DeviceContext1> DeviceContext;

    
    ComPtr<IDXGIDevice1> DXGIDevice;
    ComPtr<IDXGIAdapter> DXGIAdapter;
    ComPtr<IDXGIFactory2> DXGIFactory;

    framebufferHandle SwapchainFramebuffer;

    std::shared_ptr<commandBuffer> CommandBuffer;

    stageBuffer StageBuffer;
};

}