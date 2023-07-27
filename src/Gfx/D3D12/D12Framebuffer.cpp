#include "../Include/Context.h"
#include "../Include/Framebuffer.h"

#include "D12Framebuffer.h"
#include "D12Common.h"
#include "D12Context.h"
#include "D12Mapping.h"

#include <d3dx12.h>

namespace gfx
{

void d3d12FramebufferData::AddRenderTarget(format Format, const f32 *ClearValues)
{
    GET_CONTEXT(D12Data, context::Get());

    D3D12_RESOURCE_DESC desc = {};
    desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    desc.Alignment = 0;
    desc.Width = Framebuffer->Width;
    desc.Height = Framebuffer->Height;
    desc.DepthOrArraySize = 1;
    desc.MipLevels = 1;
    desc.Format = FormatToNative(Format);
    desc.SampleDesc.Count = 1;
    desc.SampleDesc.Quality = 0;
    desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
    desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;

    D3D12_HEAP_PROPERTIES heapProperties = {};
    heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

    D3D12_CLEAR_VALUE clearValue = {};
    clearValue.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    clearValue.Color[0] = ClearValues[0];
    clearValue.Color[1] = ClearValues[1];
    clearValue.Color[2] = ClearValues[2];
    clearValue.Color[3] = ClearValues[3];
    
    D12Data->Device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_RENDER_TARGET, &clearValue, IID_PPV_ARGS(&RenderTargets[RenderTargetsCount]));    
    
    this->RenderTargetsSRV[RenderTargetsCount] = context::Get()->ResourceManager.Images.ObtainResource();
    context::Get()->GetImage(this->RenderTargetsSRV[RenderTargetsCount])->Init(Framebuffer->Width, Framebuffer->Height, Format, imageUsage::COLOR_ATTACHMENT, memoryUsage::GpuOnly);
    
    this->ColorFormats[RenderTargetsCount++] = Format;
}

void d3d12FramebufferData::CreateHeaps()
{
    assert(RenderTargetsCount>0);
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(context::Get()->ApiContextData);

    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = RenderTargetsCount + (IsMultiSampled ? 1 : 0);
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(D12Data->Device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&RenderTargetViewHeap)));
    RTVDescriptorSize = D12Data->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        

    D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc;
    dsvHeapDesc.NumDescriptors = 1 + (IsMultiSampled ? 1 : 0);  //Un seul depth buffer
    dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;  //Depth/stencil view type
    dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;   
    dsvHeapDesc.NodeMask = 0;  
    ThrowIfFailed(D12Data->Device->CreateDescriptorHeap(&dsvHeapDesc,IID_PPV_ARGS(&DepthBufferViewHeap)));         
    DSVDescriptorSize = D12Data->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void d3d12FramebufferData::CreateDescriptors()
{
    std::shared_ptr<d3d12Data> D12Data = std::static_pointer_cast<d3d12Data>(context::Get()->ApiContextData);

    // Create a RTV for each frame.
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(RenderTargetViewHeap->GetCPUDescriptorHandleForHeapStart());
    for (UINT n = 0; n < RenderTargetsCount; n++)
    {
        D12Data->Device->CreateRenderTargetView(RenderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, RTVDescriptorSize);
    }   

    if (IsSwapchain && IsMultiSampled )
    {
        D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {};
        rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DMS;  // Use multisample view
        D12Data->Device->CreateRenderTargetView(MultisampledColorImage.Get(), &rtvDesc, rtvHandle);
        MultisampledColorImageIndex = RenderTargetsCount;
    }
}

void d3d12FramebufferData::SetSwapchainRenderTargets(ComPtr<ID3D12Resource> *Buffers, u32 Count, format Format)
{
    this->IsSwapchain=true;
    this->RenderTargetsCount = Count;

    for (UINT n = 0; n < Count; n++)
    {
        this->RenderTargets[n] = Buffers[n];
        this->ColorFormats[n]= Format;
        
        this->RenderTargetsSRV[n] = context::Get()->ResourceManager.Images.ObtainResource();
        context::Get()->GetImage(this->RenderTargetsSRV[n])->Init(Framebuffer->Width, Framebuffer->Height, ColorFormats[n], imageUsage::COLOR_ATTACHMENT, memoryUsage::GpuOnly);
    }        
}


void d3d12FramebufferData::CreateDepthBuffer(u32 Width, u32 Height, format Format)
{
    this->DepthStencilFormat = FormatToNative(Format);

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
                                                        D3D12_HEAP_FLAG_NONE,  &depthStencilDesc,  D3D12_RESOURCE_STATE_DEPTH_WRITE,  
                                                        &optClear,  IID_PPV_ARGS(DepthStencilBuffer.GetAddressOf())));   

    CD3DX12_CPU_DESCRIPTOR_HANDLE DSVHandle(DepthBufferViewHeap->GetCPUDescriptorHandleForHeapStart());

    D12Data->Device->CreateDepthStencilView( DepthStencilBuffer.Get(),  nullptr, DSVHandle);
    DSVHandle.Offset(1, DSVDescriptorSize);

    if(IsSwapchain && IsMultiSampled)
    {
        D3D12_DEPTH_STENCIL_VIEW_DESC DSVDesc = {};
        DSVDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DMS;  // Use multisample view
        D12Data->Device->CreateDepthStencilView(MultisampledDepthImage.Get(), &DSVDesc, DSVHandle);
        MultisampledDepthImageIndex = 1;
    }
                                                             
}

}