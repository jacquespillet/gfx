#include "../../App/App.h"
#include "../Include/Context.h"
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
#include "D12Context.h"
#include "D12Common.h"
#include "D12Swapchain.h"
#include "D12Buffer.h"
#include "D12Image.h"
#include "D12Pipeline.h"
#include "D12Framebuffer.h"
#include "D12Mapping.h"
#include "D12CommandBuffer.h"
#include "D12Uniform.h"

#include <iostream>

#include <wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dx12.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#include <dxc/dxcapi.h>
#include <d3d12shader.h> // Contains functions and structures useful in accessing shader information.

// #include <dxc/DxilContainer/DxilContainer.h>
// #include <dxc/DxilContainer/DxilContainerUtil.h>
// #include <dxc/DxilShader/DxilShaderModel.h>
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
            if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, _uuidof(ID3D12Device), nullptr)))
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
    D12Data->Debug = InitializeInfo.Debug;
    D12Data->MultisamplingEnabled = InitializeInfo.EnableMultisampling;
    
    Singleton->Window = &Window;
    UINT dxgiFactoryFlags = 0;

    // Enable the debug layer (requires the Graphics Tools "optional feature").
    // NOTE: Enabling the debug layer after device creation will invalidate the active device.
	if(D12Data->Debug)
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
            dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
        }
    }

    
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&D12Data->Factory)));

    ComPtr<IDXGIAdapter1> hardwareAdapter;
    GetHardwareAdapter(D12Data->Factory.Get(), &hardwareAdapter);

    ThrowIfFailed(D3D12CreateDevice(
        hardwareAdapter.Get(),
        D3D_FEATURE_LEVEL_11_0,
        IID_PPV_ARGS(&D12Data->Device)
        ));

    D3D12_DESCRIPTOR_HEAP_DESC HeapDesc;
    HeapDesc.NumDescriptors = 1024;
    HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    HeapDesc.NodeMask = 0;
    ThrowIfFailed(D12Data->Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&D12Data->CommonDescriptorHeap)));
    D12Data->DescriptorSize = D12Data->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(D12Data->Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&D12Data->CommandQueue))); 

    ThrowIfFailed(D12Data->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&D12Data->ImmediateCommandAllocator)));
    D12Data->ImmediateCommandBuffer = CreateD3D12CommandBuffer(D12Data->ImmediateCommandAllocator);

    D12Data->StageBuffer = Singleton->CreateStageBuffer(InitializeInfo.MaxStageBufferSize);

    //Create a fence initialized with 0
    ThrowIfFailed(D12Data->Device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&D12Data->ImmediateFence)));
    
    //Create a fence event that will be triggered
    D12Data->ImmediateFenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (D12Data->ImmediateFenceEvent == nullptr)
    {
        ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
    }

    return Singleton;
}


stageBuffer context::CreateStageBuffer(sz Size)
{
    stageBuffer Result;
    Result.Init(Size);
    return Result;
}

bufferHandle context::CreateBuffer(sz Size, bufferUsage::value Usage, memoryUsage MemoryUsage, sz Stride)
{
    bufferHandle Handle = ResourceManager.Buffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }
    buffer *Buffer = GetBuffer(Handle);
    Buffer->ApiData = std::make_shared<d3d12BufferData>();
    GET_API_DATA(D12BufferData, d3d12BufferData, Buffer);
    
    Buffer->Init(Size, Stride, Usage, MemoryUsage);
    return Handle;
}


std::shared_ptr<swapchain> context::CreateSwapchain(u32 Width, u32 Height, std::shared_ptr<swapchain> OldSwapchain)
{
    GET_CONTEXT(D12Data, this);    
    
    format SwapchainFormat = format::R8G8B8A8_UNORM;
    format DepthFormat = format::D24_UNORM_S8_UINT;
    
    std::shared_ptr<swapchain> Swapchain = std::make_shared<swapchain>();
    Swapchain->Width = Width;
    Swapchain->Height = Height;
    Swapchain->ApiData = std::make_shared<d3d12SwapchainData>();
    GET_API_DATA(D12SwapchainData, d3d12SwapchainData, Swapchain);


    D12SwapchainData->FramebufferHandle = ResourceManager.Framebuffers.ObtainResource();
    if(D12SwapchainData->FramebufferHandle == InvalidHandle)
    {
        assert(false);
        return nullptr;
    }
    framebuffer *Framebuffer = GetFramebuffer(D12SwapchainData->FramebufferHandle);
    Framebuffer->Width = Width;
    Framebuffer->Height = Height;
    Framebuffer->ApiData = std::make_shared<d3d12FramebufferData>();
    std::shared_ptr<d3d12FramebufferData> D12FramebufferData = std::static_pointer_cast<d3d12FramebufferData>(Framebuffer->ApiData);
    D12FramebufferData->Framebuffer = Framebuffer;

    Framebuffer->RenderPass = context::Get()->ResourceManager.RenderPasses.ObtainResource();
    renderPass *RenderPass = context::Get()->GetRenderPass(Framebuffer->RenderPass);
    RenderPass->Output.NumColorFormats = 1;
    RenderPass->Output.ColorFormats[0] = SwapchainFormat;
    RenderPass->Output.DepthStencilFormat = DepthFormat;
    RenderPass->Output.SampleCount = D12Data->MultisamplingEnabled ? 4 : 1;
    SwapchainOutput = RenderPass->Output;

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.BufferCount = d12Constants::FrameCount;
    swapChainDesc.Width = Width;
    swapChainDesc.Height = Height;
    swapChainDesc.Format = FormatToNative(SwapchainFormat);
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;

    

    ComPtr<IDXGISwapChain1> swapChain;
    ThrowIfFailed(D12Data->Factory->CreateSwapChainForHwnd(
        D12Data->CommandQueue.Get(),        // Swap chain needs the queue so that it can force a flush on it.
        Window->GetNativeWindow(),
        &swapChainDesc,
        nullptr,
        nullptr,
        &swapChain
        ));

    ThrowIfFailed(D12Data->Factory->MakeWindowAssociation(Window->GetNativeWindow(), DXGI_MWA_NO_ALT_ENTER));
    ThrowIfFailed(swapChain.As(&D12SwapchainData->SwapChain));
    D12SwapchainData->SetFrameIndex(D12SwapchainData->SwapChain->GetCurrentBackBufferIndex());
    
    //Retrieve buffers of the swapchain and store them
    for (UINT n = 0; n < d12Constants::FrameCount; n++)
    {
        ThrowIfFailed(D12SwapchainData->SwapChain->GetBuffer(n, IID_PPV_ARGS(&D12SwapchainData->Buffers[n])));
    }

    if (D12Data->MultisamplingEnabled)
    {

        //Create color multisampled texture
        D3D12_RESOURCE_DESC textureDesc = {};
        textureDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        textureDesc.Width = Width;
        textureDesc.Height = Height;
        textureDesc.Format = FormatToNative(SwapchainFormat);
        textureDesc.SampleDesc.Count = 4;  // Number of samples per pixel
        textureDesc.SampleDesc.Quality = 0;  // Default quality level
        textureDesc.DepthOrArraySize = 1;
        textureDesc.MipLevels=1;
        textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        D3D12_HEAP_PROPERTIES heapProperties = {};
        heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;

        D3D12_CLEAR_VALUE ClearColors = {};
        ClearColors.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        ClearColors.Color[0] = 0.5f;
        ClearColors.Color[1] = 0.0f;
        ClearColors.Color[2] = 0.8f;
        ClearColors.Color[3] = 1.0f;
        D12Data->Device->CreateCommittedResource(
            &heapProperties,  // Desired heap properties
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            &ClearColors,
            IID_PPV_ARGS(&D12FramebufferData->MultisampledColorImage)
        );

        //Create depth multisampled texture
        textureDesc.Format = FormatToNative(DepthFormat);
        textureDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        D3D12_CLEAR_VALUE ClearDepthStencil;
        ClearDepthStencil.Format = textureDesc.Format;
        ClearDepthStencil.DepthStencil.Depth = 1.0f;
        ClearDepthStencil.DepthStencil.Stencil = 0;

        D12Data->Device->CreateCommittedResource(
            &heapProperties,  // Desired heap properties
            D3D12_HEAP_FLAG_NONE,
            &textureDesc,
            D3D12_RESOURCE_STATE_DEPTH_WRITE,
            &ClearDepthStencil,
            IID_PPV_ARGS(&D12FramebufferData->MultisampledDepthImage)
        );
    }

    D12FramebufferData->IsMultiSampled = D12Data->MultisamplingEnabled;
    D12FramebufferData->SetSwapchainRenderTargets(D12SwapchainData->Buffers, d12Constants::FrameCount, SwapchainFormat);
    D12FramebufferData->CreateHeaps();
    D12FramebufferData->CreateDepthBuffer(Width, Height, DepthFormat);
    D12FramebufferData->CreateDescriptors();

    this->Swapchain = Swapchain;
    
    D12Data->VirtualFrames.Init();
    
    return Swapchain;
}

bufferHandle CreateVertexBufferStream(f32 *Values, sz ByteSize, sz Stride, const std::vector<vertexInputAttribute> &Attributes)
{
    context *Context = context::Get();
    GET_CONTEXT(D12Data, Context);
    bufferHandle Handle = Context->ResourceManager.Buffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }

    //Create vertex buffer
    buffer *Buffer = Context->GetBuffer(Handle);
    Buffer->Init(ByteSize, 1, bufferUsage::VertexBuffer, memoryUsage::GpuOnly);
    Buffer->Name = "";
    GET_API_DATA(D12BufferData, d3d12BufferData, Buffer);    

    //Copy data to stage buffer
    D12Data->ImmediateCommandBuffer->Begin();
    auto VertexAllocation = D12Data->StageBuffer.Submit((uint8_t*)Values, (u32)ByteSize);
    
    //Copy stage buffer to vertex buffer
    D12Data->ImmediateCommandBuffer->CopyBuffer(
        bufferInfo {D12Data->StageBuffer.GetBuffer(), VertexAllocation.Offset},
        bufferInfo {Buffer, 0},
        VertexAllocation.Size
    );
    
    //Submit command buffer
    D12Data->StageBuffer.Flush();
    D12Data->ImmediateCommandBuffer->End();
    Context->SubmitCommandBufferImmediate(D12Data->ImmediateCommandBuffer.get());
    D12Data->StageBuffer.Reset(); 

    // Initialize the vertex buffer view.
    D12BufferData->VertexBufferView.BufferLocation = D12BufferData->Handle->GetGPUVirtualAddress();
    D12BufferData->VertexBufferView.StrideInBytes = (u32)Stride;
    D12BufferData->VertexBufferView.SizeInBytes = (u32)ByteSize;   
    
    return Handle;
}


vertexBufferHandle context::CreateVertexBuffer(const vertexBufferCreateInfo &CreateInfo)
{
    GET_CONTEXT(VkData, context::Get());
    
    vertexBufferHandle Handle = this->ResourceManager.VertexBuffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        assert(false);
        return Handle;
    }
    vertexBuffer *VertexBuffer = GetVertexBuffer(Handle);
    VertexBuffer->NumVertexStreams = CreateInfo.NumVertexStreams;
    memcpy(&VertexBuffer->VertexStreams[0], &CreateInfo.VertexStreams[0], commonConstants::MaxVertexStreams * sizeof(vertexStreamData));
    
    for(sz i=0; i<VertexBuffer->NumVertexStreams; i++)
    {
        std::vector<vertexInputAttribute> Attributes(VertexBuffer->VertexStreams[i].AttributesCount);
        memcpy(&Attributes[0], &CreateInfo.VertexStreams[i].InputAttributes, VertexBuffer->VertexStreams[i].AttributesCount * sizeof(vertexInputAttribute));

        VertexBuffer->VertexStreams[i].Buffer = CreateVertexBufferStream((f32*)VertexBuffer->VertexStreams[i].Data, VertexBuffer->VertexStreams[i].Size, VertexBuffer->VertexStreams[i].Stride, Attributes);
    }

    return Handle;
}


void context::SubmitCommandBufferImmediate(commandBuffer *CommandBuffer)
{
    GET_CONTEXT(D12Data, this);
    
    D12Data->ImmediateFence->Signal(0);


    std::shared_ptr<d3d12CommandBufferData> D12CommandData = std::static_pointer_cast<d3d12CommandBufferData>(CommandBuffer->ApiData);

    // Execute the initialization commands.
	ID3D12CommandList* CommandLists[] = { D12CommandData->CommandList.Get() };
	D12Data->CommandQueue->ExecuteCommandLists(1, CommandLists);

    ThrowIfFailed(D12Data->CommandQueue->Signal(D12Data->ImmediateFence.Get(), 1));
    ThrowIfFailed(D12Data->ImmediateFence->SetEventOnCompletion(1, D12Data->ImmediateFenceEvent));
    WaitForSingleObjectEx(D12Data->ImmediateFenceEvent, INFINITE, FALSE);
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
    Framebuffer->ApiData = std::make_shared<d3d12FramebufferData>();
    GET_API_DATA(D12FramebufferData, d3d12FramebufferData, Framebuffer);
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
    D12FramebufferData->CreateHeaps();
    D12FramebufferData->CreateDepthBuffer(CreateInfo.Width, CreateInfo.Height, CreateInfo.DepthFormat);
    D12FramebufferData->CreateDescriptors();


    return FramebufferHandle;
    
}

ComPtr<IDxcBlob> CompileShader(const shaderStage &Stage, std::vector<D3D12_ROOT_PARAMETER> &OutRootParams, std::unordered_map<u32, u32> &BindingRootParamMapping, std::vector<CD3DX12_DESCRIPTOR_RANGE> &DescriptorRanges)
{
    GET_CONTEXT(D12Data, context::Get());

    ComPtr<IDxcUtils> Utils;
    ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&Utils)));
    ComPtr<IDxcCompiler3> Compiler;
    ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler)));
    ComPtr<IDxcIncludeHandler> IncludeHandler;
    ThrowIfFailed(Utils->CreateDefaultIncludeHandler(&IncludeHandler));

    std::vector<LPCWSTR> CompileArgs =
    {
        DXC_ARG_PACK_MATRIX_COLUMN_MAJOR,
        // DXC_ARG_WARNINGS_ARE_ERRORS,
        DXC_ARG_ALL_RESOURCES_BOUND
    };

    if(Stage.Stage == shaderStageFlags::Vertex)
    {
        CompileArgs.push_back(L"-E");
        CompileArgs.push_back(L"VSMain");
        CompileArgs.push_back(L"-T");
        CompileArgs.push_back(L"vs_6_0");
    }
    else if(Stage.Stage ==  shaderStageFlags::Fragment)
    {
        CompileArgs.push_back(L"-E");
        CompileArgs.push_back(L"PSMain");
        CompileArgs.push_back(L"-T");
        CompileArgs.push_back(L"ps_6_0");
    }
    else if(Stage.Stage ==  shaderStageFlags::Compute)
    {
        CompileArgs.push_back(L"-E");
        CompileArgs.push_back(L"CSMain");
        CompileArgs.push_back(L"-T");
        CompileArgs.push_back(L"cs_6_0");
    }

    
    if(D12Data->Debug)
    {
        CompileArgs.push_back(DXC_ARG_DEBUG);
    }
    else
    {
        CompileArgs.push_back(DXC_ARG_OPTIMIZATION_LEVEL3);
    }    

    // Load the shader source file to a blob.
    ComPtr<IDxcBlobEncoding> sourceBlob{};
    ThrowIfFailed(Utils->LoadFile(ConstCharToLPCWSTR(Stage.FileName), nullptr, &sourceBlob));

    DxcBuffer sourceBuffer
    {
        sourceBlob->GetBufferPointer(),
        sourceBlob->GetBufferSize(),
        0u,
    };

    // Compile the shader.
    Microsoft::WRL::ComPtr<IDxcResult> CompiledShaderBuffer{};
    const HRESULT hr = Compiler->Compile(&sourceBuffer,
                            CompileArgs.data(),
                            static_cast<uint32_t>(CompileArgs.size()),
                            IncludeHandler.Get(),   
                            IID_PPV_ARGS(&CompiledShaderBuffer));
    if (FAILED(hr))
    {
        assert(false);
    }
    ComPtr<IDxcBlob> CompiledShaderBlob{nullptr};
    CompiledShaderBuffer->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&CompiledShaderBlob), nullptr);

    // Get compilation errors (if any).
    ComPtr<IDxcBlobUtf8> errors{};
    ThrowIfFailed(CompiledShaderBuffer->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr));
    if (errors && errors->GetStringLength() > 0)
    {
        const LPCSTR errorMessage = errors->GetStringPointer();
        std::wcout << LPCSTRToWString(errorMessage) << std::endl;
        //assert(false);
    }    

    // Get shader reflection data.
    ComPtr<IDxcBlob> reflectionBlob{};
    ThrowIfFailed(CompiledShaderBuffer->GetOutput(DXC_OUT_REFLECTION, IID_PPV_ARGS(&reflectionBlob), nullptr));

    const DxcBuffer reflectionBuffer
    {
        reflectionBlob->GetBufferPointer(),
        reflectionBlob->GetBufferSize(),
        0,
    };

    ComPtr<ID3D12ShaderReflection> shaderReflection{};
    Utils->CreateReflection(&reflectionBuffer, IID_PPV_ARGS(&shaderReflection));
    D3D12_SHADER_DESC shaderDesc{};
    shaderReflection->GetDesc(&shaderDesc);    
    
    
    for (uint32_t i=0; i<shaderDesc.BoundResources; i++)
    {
        D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc{};
        ThrowIfFailed(shaderReflection->GetResourceBindingDesc(i, &shaderInputBindDesc));
        if (shaderInputBindDesc.Type == D3D_SIT_CBUFFER)
        {
            BindingRootParamMapping[shaderInputBindDesc.BindPoint] = static_cast<uint32_t>(OutRootParams.size());

            D3D12_ROOT_PARAMETER rootParameter = {};
            rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            rootParameter.Descriptor = {};
            rootParameter.Descriptor.ShaderRegister = shaderInputBindDesc.BindPoint,
            rootParameter.Descriptor.RegisterSpace = shaderInputBindDesc.Space,
            // rootParameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
            
            OutRootParams.push_back(rootParameter);
        }
        else if (shaderInputBindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED)
        {
            BindingRootParamMapping[shaderInputBindDesc.BindPoint] = static_cast<uint32_t>(OutRootParams.size());

            D3D12_ROOT_PARAMETER rootParameter = {};
            rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            rootParameter.Descriptor = {};
            rootParameter.Descriptor.ShaderRegister = shaderInputBindDesc.BindPoint,
            rootParameter.Descriptor.RegisterSpace = shaderInputBindDesc.Space,
            // rootParameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
            
            OutRootParams.push_back(rootParameter);
        }
        else if (shaderInputBindDesc.Type == D3D_SIT_TEXTURE)
        {
            // For now, each individual texture belongs in its own descriptor table. This can cause the root signature to quickly exceed the 64WORD size limit.
            BindingRootParamMapping[shaderInputBindDesc.BindPoint] = static_cast<uint32_t>(OutRootParams.size());
            const CD3DX12_DESCRIPTOR_RANGE srvRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
                                    1u,
                                    shaderInputBindDesc.BindPoint,
                                    shaderInputBindDesc.Space,
                                    D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND);
            
            DescriptorRanges.push_back(srvRange);

            D3D12_ROOT_PARAMETER RootParameter = {};
            RootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
            RootParameter.DescriptorTable = {};
            RootParameter.DescriptorTable.NumDescriptorRanges = 1u,
            RootParameter.DescriptorTable.pDescriptorRanges = &DescriptorRanges.back(),
            RootParameter.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL,
            
            OutRootParams.push_back(RootParameter);  
        }
    } 
    
    return CompiledShaderBlob;
}

pipelineHandle context::RecreatePipeline(const pipelineCreation &PipelineCreation, pipelineHandle PipelineHandle)
{
    GET_CONTEXT(D12Data, this);
    pipeline *Pipeline = GetPipeline(PipelineHandle);
    Pipeline->Name = std::string(PipelineCreation.Name);
    Pipeline->Creation = PipelineCreation;    
    GET_API_DATA(D12PipelineData, d3d12PipelineData, Pipeline);
    D12PipelineData->DestroyD12Resources();
    D12PipelineData->Create(PipelineCreation); 
    return PipelineHandle;
}

pipelineHandle context::CreatePipeline(const pipelineCreation &PipelineCreation)
{
    pipelineHandle Handle = ResourceManager.Pipelines.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }
    pipeline *Pipeline = GetPipeline(Handle);
    Pipeline->ApiData = std::make_shared<d3d12PipelineData>();
    std::shared_ptr<d3d12PipelineData> D12PipelineData = std::static_pointer_cast<d3d12PipelineData>(Pipeline->ApiData);
    D12PipelineData->Create(PipelineCreation);
   
    return Handle;
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

imageHandle context::CreateImageArray(u32 Width, u32 Height, u32 Depth, format Format, imageUsage::bits Usage)
{
    imageHandle ImageHandle = ResourceManager.Images.ObtainResource();
    if(ImageHandle == InvalidHandle)
    {
        return ImageHandle;
    }
    image *Image = GetImage(ImageHandle);
    *Image = image();
    Image->InitAsArray(Width, Height, Depth, Format, Usage, memoryUsage::GpuOnly);
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


renderPassHandle context::GetDefaultRenderPass()
{
    return 0;
}

framebufferHandle context::GetSwapchainFramebuffer()
{
    GET_API_DATA(D12SwapchainData, d3d12SwapchainData, Swapchain);
    return D12SwapchainData->FramebufferHandle;
}  
  


std::shared_ptr<commandBuffer> context::GetCurrentFrameCommandBuffer()
{
    GET_CONTEXT(D12Data, this);
    return D12Data->VirtualFrames.CommandBuffer;
}

void context::OnResize(u32 NewWidth, u32 NewHeight)
{
    //TODO: Does it just work ? idk
    //No it doesn't : it doesn't resize the back buffers, so they get interpolated instead.
}

void context::Present()
{

}
void context::BindUniformsToPipeline(std::shared_ptr<uniformGroup> Uniforms, pipelineHandle PipelineHandle, u32 Binding, b8 Force){
//Uniforms don't need to know about the pipeline
}

void context::EndFrame()
{
    GET_CONTEXT(D12Data, this);
    D12Data->VirtualFrames.EndFrame();
    
    ProcessDeletionQueue();
}

void context::StartFrame()
{
    GET_CONTEXT(D12Data, this);
    D12Data->VirtualFrames.StartFrame();
}

void context::CopyDataToBuffer(bufferHandle BufferHandle, void *Ptr, sz Size, sz Offset)
{
    GET_CONTEXT(D12Data, this);
    
    buffer *Buffer = GetBuffer(BufferHandle);
    Buffer->CopyData((u8*)Ptr, Size, Offset);
}

void context::DestroyVertexBuffer(bufferHandle VertexBufferHandle)
{
    vertexBuffer *VertexBuffer = GetVertexBuffer(VertexBufferHandle);
    for (sz i = 0; i < VertexBuffer->NumVertexStreams; i++)
    {
        DestroyBuffer(VertexBuffer->VertexStreams[i].Buffer);
    }
    ResourceManager.VertexBuffers.ReleaseResource(VertexBufferHandle);
}

void context::DestroyPipeline(pipelineHandle PipelineHandle)
{
    pipeline *Pipeline = GetPipeline(PipelineHandle);
    GET_API_DATA(D12Pipeline, d3d12PipelineData, Pipeline);
    D12Pipeline->RootSignature.Reset();
    D12Pipeline->PipelineState.Reset();
    if(D12Pipeline->vertexShader) D12Pipeline->vertexShader.Reset();
    if(D12Pipeline->pixelShader) D12Pipeline->pixelShader.Reset();
    if(D12Pipeline->computeShader) D12Pipeline->computeShader.Reset();

    ResourceManager.Pipelines.ReleaseResource(PipelineHandle);
}
void context::DestroyBuffer(bufferHandle BufferHandle)
{
    buffer *Buffer = GetBuffer(BufferHandle);
    GET_API_DATA(D12Buffer, d3d12BufferData, Buffer);
    if(Buffer->MappedData != nullptr) Buffer->UnmapMemory();

    
    D12Buffer->Handle.Reset();

    ResourceManager.Buffers.ReleaseResource(BufferHandle);
}
void context::DestroyImage(imageHandle ImageHandle)
{
    image *Image = GetImage(ImageHandle);
    if(!Image) return;
    GET_API_DATA(D12Image, d3d12ImageData, Image);
    D12Image->Handle.Reset();

    ResourceManager.Images.ReleaseResource(ImageHandle);
}
void context::DestroyFramebuffer(framebufferHandle FramebufferHandle)
{
    framebuffer *Framebuffer = GetFramebuffer(FramebufferHandle);
    GET_API_DATA(D12Framebuffer, d3d12FramebufferData, Framebuffer);

    D12Framebuffer->RenderTargetViewHeap.Reset();
    D12Framebuffer->DepthBufferViewHeap.Reset();
    D12Framebuffer->DepthStencilBuffer.Reset();

    if(D12Framebuffer->IsMultiSampled)
    {
        D12Framebuffer->MultisampledColorImage.Reset();
        D12Framebuffer->MultisampledDepthImage.Reset();
    }
    for (size_t i = 0; i < D12Framebuffer->RenderTargetsCount; i++)
    {
        D12Framebuffer->RenderTargets[i].Reset();
        ResourceManager.Images.ReleaseResource(D12Framebuffer->RenderTargetsSRV[i]);
    }
    
    ResourceManager.RenderPasses.ReleaseResource(Framebuffer->RenderPass);
    ResourceManager.Framebuffers.ReleaseResource(FramebufferHandle);
}
void context::DestroySwapchain()
{
    GET_API_DATA(D12SwapchainData, d3d12SwapchainData, Swapchain);
    framebuffer *Framebuffer = GetFramebuffer(D12SwapchainData->FramebufferHandle);
    GET_API_DATA(D12Framebuffer, d3d12FramebufferData, Framebuffer);
    D12SwapchainData->SwapChain.Reset();
    for (size_t i = 0; i < D12Framebuffer->RenderTargetsCount; i++)
    {
        ResourceManager.Images.ReleaseResource(D12Framebuffer->RenderTargetsSRV[i]);
    }
    
    for (sz i = 0; i < d12Constants::FrameCount; i++)
    {
        D12SwapchainData->Buffers[i].Reset();
    }
    
    
    ResourceManager.RenderPasses.ReleaseResource(Framebuffer->RenderPass);
    ResourceManager.Framebuffers.ReleaseResource(D12SwapchainData->FramebufferHandle);
}

void context::WaitIdle()
{
    GET_CONTEXT(D12Data, this);
    D12Data->VirtualFrames.WaitForPreviousFrame();
}

void context::Cleanup()
{   
    GET_CONTEXT(D12Data, this);
    D12Data->StageBuffer.Destroy();
    ResourceManager.Destroy();
    
    GET_API_DATA(D12ImmediateCommandBuffer, d3d12CommandBufferData, D12Data->ImmediateCommandBuffer);
    D12ImmediateCommandBuffer->CommandList.Reset();

    D12Data->VirtualFrames.Destroy();

    D12Data->CommandQueue.Reset();
    D12Data->ImmediateCommandAllocator.Reset();
    
    D12Data->CommonDescriptorHeap.Reset();
    D12Data->SrvDescriptorHeap.Reset();
    D12Data->ImmediateFence.Reset();
    
    if (D12Data->Device)
    {
        ID3D12DebugDevice* pDebug = nullptr;
        if (SUCCEEDED(D12Data->Device->QueryInterface(IID_PPV_ARGS(&pDebug))))
        {
            // pDebug->ReportLiveDeviceObjects(D3D12_RLDO_SUMMARY);
            pDebug->ReportLiveDeviceObjects(D3D12_RLDO_DETAIL);
            pDebug->Release();
        }
    }
    D12Data->Factory.Reset();
    D12Data->Device.Reset();
}

D3D12_CPU_DESCRIPTOR_HANDLE d3d12Data::GetCPUDescriptorAt(sz Index)
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(CommonDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), (s32)Index, DescriptorSize);
}

D3D12_GPU_DESCRIPTOR_HANDLE d3d12Data::GetGPUDescriptorAt(sz Index)
{
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(CommonDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), (s32)Index, DescriptorSize);
}

}