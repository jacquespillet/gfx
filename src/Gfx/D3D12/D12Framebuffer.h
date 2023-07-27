#pragma once
#include "../Include/Types.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

namespace gfx
{
struct framebuffer;

struct d3d12FramebufferData
{
    framebuffer *Framebuffer;
    
    u32 RenderTargetsCount=0;
    
    //Heaps for storing the render targets
    ComPtr<ID3D12DescriptorHeap> RenderTargetViewHeap = {};
    ComPtr<ID3D12DescriptorHeap> DepthBufferViewHeap = {};
    u32 RTVDescriptorSize=0;
    u32 DSVDescriptorSize=0;
    
    
    //Actual buffers
    ComPtr<ID3D12Resource> RenderTargets[commonConstants::MaxImageOutputs] = {};
    imageHandle RenderTargetsSRV[commonConstants::MaxImageOutputs] = {};
    ComPtr<ID3D12Resource> DepthStencilBuffer;  
        
    
    //Formats
    DXGI_FORMAT DepthStencilFormat =DXGI_FORMAT_D24_UNORM_S8_UINT;
    format ColorFormats[commonConstants::MaxImageOutputs];

    //Swapchain info
    u32 CurrentTarget=0;
    b8 IsSwapchain=false;


    //Stores the multisampled buffers for msaa render targets
    b8 IsMultiSampled=false;
    ComPtr<ID3D12Resource> MultisampledColorImage;
    ComPtr<ID3D12Resource> MultisampledDepthImage;
    u32 MultisampledColorImageIndex; //Indices in the rtv and dsv heaps
    u32 MultisampledDepthImageIndex;
    
    void AddRenderTarget(format Format, const f32 *ClearValues);
    void CreateHeaps();
    void CreateDescriptors();
    void SetSwapchainRenderTargets(ComPtr<ID3D12Resource> *Buffers, u32 Count, format Format);
    void CreateDepthBuffer(u32 Width, u32 Height, format Format);
};

}