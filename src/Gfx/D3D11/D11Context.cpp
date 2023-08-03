#include "../../App/Window.h"
#include "../Include/Context.h"
#include "../Include/Framebuffer.h"
#include "../Include/Swapchain.h"
#include "../Include/CommandBuffer.h"
#include "D11Context.h"
#include "D11Common.h"
#include "D11Framebuffer.h"
#include "D11Swapchain.h"
#include "D11Pipeline.h"
#include "D11Buffer.h"
#include "D11CommandBuffer.h"

#include <d3d11_1.h>

namespace gfx
{

std::shared_ptr<context> context::Singleton = {};

context *context::Get()
{
    return Singleton.get();
}
std::shared_ptr<context> context::Initialize(initializeInfo &InitializeInfo, app::window &Window)
{
    if(Singleton==nullptr){
        Singleton = std::make_shared<context>();
    }

    Singleton->ResourceManager.Init();
    Singleton->ApiContextData = std::make_shared<d3d11Data>();
    GET_CONTEXT(D11Data, Singleton);
    
    Singleton->Window = &Window;
    
    
    //Create device
    D3D_FEATURE_LEVEL FeatureLevels[] = { D3D_FEATURE_LEVEL_11_1 };
    D3D11CreateDevice(nullptr, 
                      D3D_DRIVER_TYPE_HARDWARE, 
                      nullptr, 
                      D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG, 
                      FeatureLevels, 
                      ARRAYSIZE(FeatureLevels), 
                      D3D11_SDK_VERSION, 
                      D11Data->BaseDevice.GetAddressOf(), 
                      nullptr, 
                      D11Data->BaseDeviceContext.GetAddressOf());
    D11Data->BaseDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(D11Data->Device.GetAddressOf()));
    D11Data->BaseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(D11Data->DeviceContext.GetAddressOf()));


    D11Data->Device->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(D11Data->DXGIDevice.GetAddressOf()));
    D11Data->DXGIDevice->GetAdapter(D11Data->DXGIAdapter.GetAddressOf());
    D11Data->DXGIAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(D11Data->DXGIFactory.GetAddressOf()));

    D11Data->CommandBuffer = std::make_shared<commandBuffer>();
    D11Data->CommandBuffer->Initialize();

    D11Data->StageBuffer.Init(InitializeInfo.MaxStageBufferSize);

    Singleton->MultiSampleCount = InitializeInfo.EnableMultisampling ? 4 : 1;

    return Singleton;
}

std::shared_ptr<swapchain> context::CreateSwapchain(u32 Width, u32 Height, std::shared_ptr<swapchain> OldSwapchain)
{
    std::shared_ptr<swapchain> Swapchain = std::make_shared<swapchain>();
    Swapchain->ApiData = std::make_shared<d3d11Swapchain>();
    GET_API_DATA(D11Swapchain, d3d11Swapchain, Swapchain);
    GET_CONTEXT(D11Data, this);    
    
    DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
    SwapChainDesc.Width              = 0; // use window width
    SwapChainDesc.Height             = 0; // use window height
    SwapChainDesc.Format             = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    SwapChainDesc.Stereo             = FALSE;
    SwapChainDesc.SampleDesc.Count   = context::Get()->MultiSampleCount;
    SwapChainDesc.SampleDesc.Quality = 0;
    SwapChainDesc.BufferUsage        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    SwapChainDesc.BufferCount        = 2;
    SwapChainDesc.Scaling            = DXGI_SCALING_STRETCH;
    SwapChainDesc.SwapEffect         = DXGI_SWAP_EFFECT_DISCARD; // prefer DXGI_SWAP_EFFECT_FLIP_DISCARD, see Minimal D3D11 pt2 
    SwapChainDesc.AlphaMode          = DXGI_ALPHA_MODE_UNSPECIFIED;
    SwapChainDesc.Flags              = 0;

    D11Data->DXGIFactory->CreateSwapChainForHwnd(D11Data->Device.Get(), context::Get()->Window->GetNativeWindow(), &SwapChainDesc, nullptr, nullptr, D11Swapchain->Handle.GetAddressOf());    

    //Create Framebuffer
    D11Data->SwapchainFramebuffer = Singleton->ResourceManager.Framebuffers.ObtainResource();
    if(D11Data->SwapchainFramebuffer == InvalidHandle)
    {
        assert(false);
    }
    framebuffer *SwapchainFramebuffer = Singleton->GetFramebuffer(D11Data->SwapchainFramebuffer);
    SwapchainFramebuffer->ApiData = std::make_shared<d3d11FramebufferData>();
    GET_API_DATA(D11Framebuffer, d3d11FramebufferData, SwapchainFramebuffer);
    D11Framebuffer->RenderTargetCount=1;
    //Get Color
    D11Swapchain->Handle->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(D11Framebuffer->ColorHandles[0].GetAddressOf()));
    D11Data->Device->CreateRenderTargetView(D11Framebuffer->ColorHandles[0].Get(), nullptr, D11Framebuffer->ColorViews[0].GetAddressOf());
    
    //Get depth
    D3D11_TEXTURE2D_DESC DepthBufferDesc;
    D11Framebuffer->ColorHandles[0]->GetDesc(&DepthBufferDesc); // copy from framebuffer properties
    DepthBufferDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;
    DepthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    D11Data->Device->CreateTexture2D(&DepthBufferDesc, nullptr, D11Framebuffer->DepthBuffer.GetAddressOf());
    D11Data->Device->CreateDepthStencilView(D11Framebuffer->DepthBuffer.Get(), nullptr, D11Framebuffer->DepthBufferView.GetAddressOf());    


    SwapchainFramebuffer->Width = Width;
    SwapchainFramebuffer->Height = Height;
    
    Swapchain->Width = Width;
    Swapchain->Height = Height;

    this->Swapchain = Swapchain;
    return Swapchain;
}

imageHandle context::CreateImage(const imageData &ImageData, const imageCreateInfo& CreateInfo)
{
    imageHandle ImageHandle = ResourceManager.Images.ObtainResource();
    if(ImageHandle == InvalidHandle)
    {
        return ImageHandle;
    }
    image *Image = GetImage(ImageHandle);
    *Image = image();
    Image->Init(ImageData, CreateInfo);
    return ImageHandle;
}

imageHandle context::CreateImageCubemap(const imageData &Left, const imageData &Right, const imageData &Top, const imageData &Bottom, const imageData &Back, const imageData &Front, const imageCreateInfo& CreateInfo)
{
    imageHandle ImageHandle = ResourceManager.Images.ObtainResource();
    if(ImageHandle == InvalidHandle)
    {
        return ImageHandle;
    }
    image *Image = GetImage(ImageHandle);
    *Image = image();
    Image->InitAsCubemap(Left, Right, Top, Bottom, Back, Front, CreateInfo);
    return ImageHandle;
}


pipelineHandle context::RecreatePipeline(const pipelineCreation &PipelineCreation, pipelineHandle PipelineHandle)
{
    GET_CONTEXT(GLData, this);
    pipeline *Pipeline = GetPipeline(PipelineHandle);
    Pipeline->Name = std::string(PipelineCreation.Name);
    Pipeline->Creation = PipelineCreation;    
    GET_API_DATA(D11Pipeline, d3d11Pipeline, Pipeline);
    D11Pipeline->DestroyD11Resources();
    D11Pipeline->Create(PipelineCreation);
    return PipelineHandle;
}

pipelineHandle context::CreatePipeline(const pipelineCreation &PipelineCreation)
{
    GET_CONTEXT(D11Data, this);

    pipelineHandle Handle = ResourceManager.Pipelines.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }
    pipeline *Pipeline = GetPipeline(Handle);
    Pipeline->ApiData = std::make_shared<d3d11Pipeline>();
    Pipeline->GraphicsPipeline = !PipelineCreation.IsCompute;
    GET_API_DATA(D11Pipeline, d3d11Pipeline, Pipeline);
    
    //Compile and link shaders
    D11Pipeline->Create(PipelineCreation);
    
    return Handle;
}


framebufferHandle context::CreateFramebuffer(const framebufferCreateInfo &CreateInfo)
{
    //Create the render pass
    GET_CONTEXT(D12Data, this);
    
    framebufferHandle FramebufferHandle = ResourceManager.Framebuffers.ObtainResource();
    if(FramebufferHandle == InvalidHandle)
    {
        assert(false);
        return InvalidHandle;
    }
    framebuffer *Framebuffer = GetFramebuffer(FramebufferHandle);
    Framebuffer->ApiData = std::make_shared<d3d11FramebufferData>();
    GET_API_DATA(D12FramebufferData, d3d11FramebufferData, Framebuffer);
    D12FramebufferData->Framebuffer = Framebuffer;

    Framebuffer->Width = CreateInfo.Width;
    Framebuffer->Height = CreateInfo.Height;
    Framebuffer->RenderPass = context::Get()->ResourceManager.RenderPasses.ObtainResource();

    renderPass *RenderPass = context::Get()->GetRenderPass(Framebuffer->RenderPass);
    RenderPass->Output.NumColorFormats = (u32)CreateInfo.ColorFormats.size();
    RenderPass->Output.DepthStencilFormat = CreateInfo.DepthFormat;
    RenderPass->Output.SampleCount=1;

    assert(CreateInfo.ColorFormats.size() < commonConstants::MaxImageOutputs);
    
    for(sz i=0; i<CreateInfo.ColorFormats.size(); i++)
    {
        RenderPass->Output.ColorFormats[i] = CreateInfo.ColorFormats[i];
        D12FramebufferData->AddRenderTarget(CreateInfo.ColorFormats[i], &CreateInfo.ClearValues[0]);
    }
    D12FramebufferData->CreateDepthBuffer(CreateInfo.Width, CreateInfo.Height, CreateInfo.DepthFormat);
    D12FramebufferData->CreateDescriptors();


    return FramebufferHandle;
    
}

vertexBufferHandle context::CreateVertexBuffer(const vertexBufferCreateInfo &CreateInfo)
{
    GET_CONTEXT(D11Data, this);

    vertexBufferHandle Handle = this->ResourceManager.VertexBuffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        assert(false);
        return Handle;
    }
    
    vertexBuffer *VertexBuffer = GetVertexBuffer(Handle);
    VertexBuffer->NumVertexStreams = CreateInfo.NumVertexStreams;
    memcpy(&VertexBuffer->VertexStreams[0], &CreateInfo.VertexStreams[0], commonConstants::MaxVertexStreams * sizeof(vertexStreamData));
    
    VertexBuffer->ApiData = std::make_shared<d3d11VertexBuffer>();
    GET_API_DATA(D11VertexBuffer, d3d11VertexBuffer, VertexBuffer);
    
    for (sz i = 0; i < CreateInfo.NumVertexStreams; i++)
    {
        D11VertexBuffer->VertexBuffers[i] = context::Get()->ResourceManager.Buffers.ObtainResource();
        buffer *Buffer = context::Get()->GetBuffer(D11VertexBuffer->VertexBuffers[i]);
        Buffer->ApiData = std::make_shared<d3d11Buffer>();

        Buffer->Init(VertexBuffer->VertexStreams[i].Size, VertexBuffer->VertexStreams[i].Stride, bufferUsage::VertexBuffer, memoryUsage::GpuOnly);
        Buffer->CopyData((u8*)VertexBuffer->VertexStreams[i].Data, VertexBuffer->VertexStreams[i].Size, 0);
    }
    
    return Handle;
}

bufferHandle context::CreateBuffer(sz Size, bufferUsage::value Usage, memoryUsage MemoryUsage, sz Stride)
{
    bufferHandle Handle = ResourceManager.Buffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }
    buffer *Buffer = GetBuffer(Handle);
    Buffer->ApiData = std::make_shared<d3d11Buffer>();
    Buffer->Init(Size, Stride, Usage, MemoryUsage);
    return Handle;
}

void context::CopyDataToBuffer(bufferHandle BufferHandle, void *Ptr, sz Size, sz Offset)
{
    GET_CONTEXT(D12Data, this);
    
    buffer *Buffer = GetBuffer(BufferHandle);
    Buffer->CopyData((u8*)Ptr, Size, Offset);
}

std::shared_ptr<commandBuffer> context::GetCurrentFrameCommandBuffer()
{
    GET_CONTEXT(D11Data, this);
    return D11Data->CommandBuffer;
}


renderPassHandle context::GetDefaultRenderPass()
{
    return 0;
}

framebufferHandle context::GetSwapchainFramebuffer()
{
    GET_CONTEXT(D11Data, this);
    return D11Data->SwapchainFramebuffer;
}


void context::StartFrame()
{
}

void context::EndFrame()
{
    GET_CONTEXT(D11Data, this);
    D11Data->CommandBuffer->End();
}

void context::BindUniformsToPipeline(std::shared_ptr<uniformGroup> Uniforms, pipelineHandle PipelineHandle, u32 Binding, b8 Force)
{
}

void context::OnResize(u32 NewWidth, u32 NewHeight)
{
}

void context::Present()
{
    this->Swapchain->Present();
}

void context::WaitIdle()
{

}

void context::Cleanup()
{   
}

void context::DestroyPipeline(pipelineHandle PipelineHandle)
{
    assert(false);
}

void context::DestroyFramebuffer(framebufferHandle FramebufferHandle)
{
    assert(false);
}

void context::DestroyBuffer(bufferHandle BufferHandle)
{
    assert(false);
}

void context::DestroyVertexBuffer(vertexBufferHandle VertexBufferHandle)
{
    assert(false);
}


void context::DestroyImage(imageHandle ImageHandle)
{
    assert(false);
}
void context::DestroySwapchain()
{
    assert(false);
}


}