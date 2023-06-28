#pragma once

#include "../Include/GfxContext.h"
#include "../Include/Framebuffer.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

namespace gfx
{

struct d3d12FramebufferData
{
    static const u32 MaxRenderTargets = 16;

    u32 CurrentTarget=0;
    u32 RenderTargetsCount=0;
    
    ComPtr<ID3D12DescriptorHeap> RenderTargetViewHeap = {};
    u32 RTVDescriptorSize=0;
    ComPtr<ID3D12Resource> RenderTargets[2] = {};
};

}