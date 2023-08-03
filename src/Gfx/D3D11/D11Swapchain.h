#pragma once

#include <d3d11_1.h>

#include <wrl.h>
using namespace Microsoft::WRL;

namespace gfx
{
struct d3d11Swapchain
{
    ComPtr<IDXGISwapChain1> Handle;
};
}