#include "D12Framebuffer.h"
#include "D12Common.h"
#include "D12GfxContext.h"
#include <d3dx12.h>

namespace gfx
{
    
void d3d12FramebufferData::CreateHeaps()
{
    assert(RenderTargetsCount>0);
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(context::Get()->ApiContextData);

    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = RenderTargetsCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(D12Data->Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&RenderTargetViewHeap)));
    RTVDescriptorSize = D12Data->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1;  //Un seul depth buffer
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;  //Depth/stencil view type
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;   
    dsvHeapDesc.NodeMask = 0;  
    ThrowIfFailed(D12Data->Device->CreateDescriptorHeap(&dsvHeapDesc,IID_PPV_ARGS(&DepthBufferViewHeap)));         
}

void d3d12FramebufferData::SetRenderTargets(ComPtr<ID3D12Resource> *Buffers)
{
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(context::Get()->ApiContextData);

    // Create a RTV for each frame.
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(RenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT n = 0; n < d3d12SwapchainData::FrameCount; n++)
    {
        this->RenderTargets[n] = Buffers[n];
        //Link each rtv heap of the framebuffer with the swapchain buffers
        D12Data->Device->CreateRenderTargetView(RenderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, RTVDescriptorSize);
    }        
}

void d3d12FramebufferData::CreateDepthBuffer(u32 Width, u32 Height)
{
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(context::Get()->ApiContextData);

    D3D12_RESOURCE_DESC depthStencilDesc;
    depthStencilDesc.Dimension =D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    depthStencilDesc.Alignment = 0;
    depthStencilDesc.Width = Width;
    depthStencilDesc.Height = Height;
    depthStencilDesc.DepthOrArraySize = 1;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.Format = DepthStencilFormat;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Layout =D3D12_TEXTURE_LAYOUT_UNKNOWN;
    depthStencilDesc.Flags =D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    
    D3D12_CLEAR_VALUE optClear;
    optClear.Format = DepthStencilFormat;
    optClear.DepthStencil.Depth = 1.0f;
    optClear.DepthStencil.Stencil = 0;
    
    ThrowIfFailed(D12Data->Device->CreateCommittedResource(  &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),  
                                                        D3D12_HEAP_FLAG_NONE,  &depthStencilDesc,  D3D12_RESOURCE_STATE_COMMON,  
                                                        &optClear,  IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf())));   

    D12Data->Device->CreateDepthStencilView( DepthStencilBuffer.Get(),  nullptr,   DepthBufferViewHeap->GetCPUDescriptorHandleForHeapStart());
                                                             
}

}