#pragma once

#include <d3d11_1.h>

namespace gfx
{
struct d3d11Swapchain
{
    IDXGISwapChain1* Handle;
};
}