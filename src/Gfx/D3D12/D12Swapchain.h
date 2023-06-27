#pragma once
#include "../Include/Types.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

namespace gfx
{
struct d3d12SwapchainData
{
    static const u32 FrameCount = 2;
    ComPtr<IDXGISwapChain3> SwapChain;    
    u32 FrameIndex;
    ComPtr<ID3D12DescriptorHeap> RenderTargetViewHeap;
    u32 RTVDescriptorSize;
    ComPtr<ID3D12Resource> RenderTargets[FrameCount];
};

}