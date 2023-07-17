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

    u32 CurrentTarget=0;
    u32 RenderTargetsCount=0;
    
    ComPtr<ID3D12DescriptorHeap> RenderTargetViewHeap = {};
    ComPtr<ID3D12DescriptorHeap> DepthBufferViewHeap = {};
    
    u32 RTVDescriptorSize=0;
    u32 DSVDescriptorSize=0;
    
    ComPtr<ID3D12Resource> RenderTargets[commonConstants::MaxImageOutputs] = {};
    ComPtr<ID3D12Resource> DepthStencilBuffer;  
        
    DXGI_FORMAT DepthStencilFormat =DXGI_FORMAT_D24_UNORM_S8_UINT;
    std::vector<DXGI_FORMAT> ColorFormats;

    b8 IsSwapchain=false;

    b8 IsMultiSampled=false;

    void CreateHeaps();
    void SetRenderTargets(ComPtr<ID3D12Resource> *Buffers, u32 Count);
    void BuildDescriptors();
    void CreateDepthBuffer(u32 Width, u32 Height, format Format);
};

}