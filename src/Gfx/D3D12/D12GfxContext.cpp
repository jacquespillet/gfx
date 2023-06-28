#include "../../App/App.h"
#include "../Include/GfxContext.h"
#include "../Include/Image.h"
#include "../Include/CommandBuffer.h"
#include "../Include/Swapchain.h"
#include "../Include/Buffer.h"
#include "../Include/Pipeline.h"
#include "../Include/Shader.h"
#include "../Include/Framebuffer.h"
#include "../Include/RenderPass.h"
#include "../Include/Framebuffer.h"
#include "../Include/Memory.h"
#include "../Common/Util.h"
#include "D12GfxContext.h"
#include "D12Common.h"
#include "D12Swapchain.h"
#include "D12Buffer.h"
#include "D12Pipeline.h"
#include "D12Framebuffer.h"

#include <iostream>

#include <wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>
using namespace DirectX;


namespace gfx
{

std::shared_ptr<context> context::Singleton = {};

context *context::Get()
{
    return Singleton.get();
}



void GetHardwareAdapter(
    IDXGIFactory1* pFactory,
    IDXGIAdapter1** ppAdapter,
    bool requestHighPerformanceAdapter=false)
{
    *ppAdapter = nullptr;

    ComPtr<IDXGIAdapter1> adapter;

    ComPtr<IDXGIFactory6> factory6;
    if (SUCCEEDED(pFactory->QueryInterface(IID_PPV_ARGS(&factory6))))
    {
        for (
            UINT adapterIndex = 0;
            SUCCEEDED(factory6->EnumAdapterByGpuPreference(
                adapterIndex,
                requestHighPerformanceAdapter == true ? DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE : DXGI_GPU_PREFERENCE_UNSPECIFIED,
                IID_PPV_ARGS(&adapter)));
            ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }

    if(adapter.Get() == nullptr)
    {
        for (UINT adapterIndex = 0; SUCCEEDED(pFactory->EnumAdapters1(adapterIndex, &adapter)); ++adapterIndex)
        {
            DXGI_ADAPTER_DESC1 desc;
            adapter->GetDesc1(&desc);

            if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
            {
                // Don't select the Basic Render Driver adapter.
                // If you want a software adapter, pass in "/warp" on the command line.
                continue;
            }

            // Check to see whether the adapter supports Direct3D 12, but don't create the
            // actual device yet.
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
            {
                break;
            }
        }
    }
    
    *ppAdapter = adapter.Detach();
}

std::shared_ptr<context> context::Initialize(initializeInfo &InitializeInfo, app::window &Window)
{
    if(Singleton==nullptr){
        Singleton = std::make_shared<context>();
    }

    Singleton->ResourceManager.Init();
    Singleton->ApiContextData = std::make_shared<d3d12Data>();
    GET_CONTEXT(D12Data, Singleton);
    
    Singleton->Window = &Window;
    UINT dxgiFactoryFlags = 0;

    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
	ComPtr<ID3D12Debug> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
	{
		debugController->EnableDebugLayer();

		// Enable additional debug layers.
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}

    
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&D12Data->Factory)));

    ComPtr<IDXGIAdapter1> hardwareAdapter;
    GetHardwareAdapter(D12Data->Factory.Get(), &hardwareAdapter);

    ThrowIfFailed(D3D12CreateDevice(
        hardwareAdapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&D12Data->Device)
        ));


    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(D12Data->Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&D12Data->CommandQueue))); 


    return Singleton;
}

std::shared_ptr<swapchain> context::CreateSwapchain(u32 Width, u32 Height)
{
    GET_CONTEXT(D12Data, this);    
    std::shared_ptr<swapchain> Swapchain = std::make_shared<swapchain>();
    Swapchain->Width = Width;
    Swapchain->Height = Height;
    Swapchain->ApiData = std::make_shared<d3d12SwapchainData>();
    std::shared_ptr<d3d12SwapchainData> D12SwapchainData = std::static_pointer_cast<d3d12SwapchainData>(Swapchain->ApiData);


    D12SwapchainData->FramebufferHandle = ResourceManager.Framebuffers.ObtainResource();
    if(D12SwapchainData->FramebufferHandle == InvalidHandle)
    {
        assert(false);
        return nullptr;
    }
    framebuffer *Framebuffer = (framebuffer*)ResourceManager.Framebuffers.GetResource(D12SwapchainData->FramebufferHandle);
    Framebuffer->Width = Width;
    Framebuffer->Height = Height;
    Framebuffer->ApiData = std::make_shared<d3d12FramebufferData>();
    std::shared_ptr<d3d12FramebufferData> D12FramebufferData = std::static_pointer_cast<d3d12FramebufferData>(Framebuffer->ApiData);


    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = d3d12SwapchainData::FrameCount;
    swapChainDesc.Width = Width;
    swapChainDesc.Height = Height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;

    

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(D12Data->Factory->CreateSwapChainForHwnd(
        D12Data->CommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Window->GetNativeWindow(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
        ));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(D12Data->Factory->MakeWindowAssociation(Window->GetNativeWindow(), DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain.As(&D12SwapchainData->SwapChain));
    D12SwapchainData->SetFrameIndex(D12SwapchainData->SwapChain->GetCurrentBackBufferIndex());
    
    //Retrieve buffers of the swapchain and store them
    for (UINT n = 0; n < d3d12SwapchainData::FrameCount; n++)
    {
        ThrowIfFailed(D12SwapchainData->SwapChain->GetBuffer(n, IID_PPV_ARGS(&D12SwapchainData->Buffers[n])));
    }
    //TODO
    //Here we create a framebuffer object
    //  It contains all the descriptor heap, size etc.. for each attachment
    //  Create depth buffer too
    //  We don't store the resources in the swapchain
    D12FramebufferData->RenderTargetsCount = d3d12SwapchainData::FrameCount;
    D12FramebufferData->CreateHeaps();
    D12FramebufferData->SetRenderTargets(D12SwapchainData->Buffers);
    D12FramebufferData->CreateDepthBuffer(Width, Height);

    this->Swapchain = Swapchain;
    
    D12Data->VirtualFrames.Init();
    
    return Swapchain;
}

struct Vertex
{
    XMFLOAT3 position;
    XMFLOAT4 color;
};

bufferHandle context::CreateVertexBuffer(f32 *Values, sz Count)
{
    GET_CONTEXT(D12Data, this);
    bufferHandle Handle = ResourceManager.Buffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }

    buffer *Buffer = (buffer*)ResourceManager.Buffers.GetResource(Handle);
    
    Buffer->Name = "";
    Buffer->ApiData = std::make_shared<d3d12BufferData>();
    std::shared_ptr<d3d12BufferData> D12BufferData = std::static_pointer_cast<d3d12BufferData>(Buffer->ApiData);
    *D12BufferData = d3d12BufferData();

    // Define the geometry for a triangle.
    Vertex triangleVertices[] =
    {
        { { 0.0f, 0.25f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { 0.25f, -0.25f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.25f, -0.25f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };

    const UINT vertexBufferSize = sizeof(triangleVertices);

    // Note: using upload heaps to transfer static data like vert buffers is not 
    // recommended. Every time the GPU needs it, the upload heap will be marshalled 
    // over. Please read up on Default Heap usage. An upload heap is used here for 
    // code simplicity and because there are very few verts to actually transfer.
    ThrowIfFailed(D12Data->Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&D12BufferData->Handle)));

    // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(D12BufferData->Handle->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
    D12BufferData->Handle->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    D12BufferData->BufferView.BufferLocation = D12BufferData->Handle->GetGPUVirtualAddress();
    D12BufferData->BufferView.StrideInBytes = sizeof(Vertex);
    D12BufferData->BufferView.SizeInBytes = vertexBufferSize;   
    
    return Handle;
}

pipelineHandle context::CreatePipeline(const pipelineCreation &PipelineCreation)
{
    pipelineHandle Handle = ResourceManager.Pipelines.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }
    pipeline *Pipeline = (pipeline*)ResourceManager.Pipelines.GetResource(Handle);
    Pipeline->ApiData = std::make_shared<d3d12PipelineData>();
    std::shared_ptr<d3d12PipelineData> D12PipelineData = std::static_pointer_cast<d3d12PipelineData>(Pipeline->ApiData);

    GET_CONTEXT(D12Data, this);

    // Create an empty root signature.
    {
        CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
        rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        ComPtr<ID3DBlob> signature;
        ComPtr<ID3DBlob> error;
        ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
        ThrowIfFailed(D12Data->Device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&D12PipelineData->RootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {
        ComPtr<ID3DBlob> vertexShader;
        ComPtr<ID3DBlob> pixelShader;

        // Enable better shader debugging with the graphics debugging tools.
        UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;

        ThrowIfFailed(D3DCompileFromFile(L"resources/Shaders/shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
        ThrowIfFailed(D3DCompileFromFile(L"resources/Shaders/shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

        // Define the vertex input layout.
        D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };

        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
        psoDesc.pRootSignature = D12PipelineData->RootSignature.Get();
        psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
        psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
        psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        psoDesc.DepthStencilState.DepthEnable = FALSE;
        psoDesc.DepthStencilState.StencilEnable = FALSE;
        psoDesc.SampleMask = UINT_MAX;
        psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        psoDesc.NumRenderTargets = 1;
        psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
        psoDesc.SampleDesc.Count = 1;
        ThrowIfFailed(D12Data->Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&D12PipelineData->PipelineState)));
    }    

    return Handle;
}

renderPassHandle context::GetDefaultRenderPass()
{
    return 0;
}

framebufferHandle context::GetSwapchainFramebuffer()
{
    return 0;
}



std::shared_ptr<commandBuffer> context::GetCurrentFrameCommandBuffer()
{
    GET_CONTEXT(D12Data, this);
    return D12Data->VirtualFrames.CommandBuffer;
}

void context::Present()
{

}

void context::EndFrame()
{
    GET_CONTEXT(D12Data, this);
    D12Data->VirtualFrames.EndFrame();
}

void context::StartFrame()
{
    GET_CONTEXT(D12Data, this);
    D12Data->VirtualFrames.StartFrame();
}
void context::Cleanup()
{

}

}