#pragma once

#include "D12Common.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

namespace gfx
{
struct d3d12SwapchainData
{
    ComPtr<IDXGISwapChain3> SwapChain;    
    framebufferHandle FramebufferHandle;
    ComPtr<ID3D12Resource> Buffers[d12Constants::FrameCount] = {};



    u32 GetFrameIndex();
    void SetFrameIndex(u32 _FrameIndex);

private:
    u32 FrameIndex;
};

}