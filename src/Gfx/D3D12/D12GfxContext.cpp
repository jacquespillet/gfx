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

    D3D12_DESCRIPTOR_HEAP_DESC HeapDesc;
    HeapDesc.NumDescriptors = 1024;
    HeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    HeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    HeapDesc.NodeMask = 0;
    D12Data->Device->CreateDescriptorHeap(&HeapDesc, IID_PPV_ARGS(&D12Data->CommonDescriptorHeap));
    D12Data->DescriptorSize = D12Data->Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(D12Data->Device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&D12Data->CommandQueue))); 

    ThrowIfFailed(D12Data->Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&D12Data->ImmediateCommandAllocator)));
    D12Data->ImmediateCommandBuffer = CreateD3D12CommandBuffer(D12Data->ImmediateCommandAllocator);

    D12Data->StageBuffer = Singleton->CreateStageBuffer(InitializeInfo.MaxStageBufferSize);


    return Singleton;
}


stageBuffer context::CreateStageBuffer(sz Size)
{
    stageBuffer Result;
    Result.Init(Size);
    return Result;
}

bufferHandle context::CreateBuffer(sz Size, bufferUsage::Bits Usage, memoryUsage MemoryUsage)
{
    bufferHandle Handle = ResourceManager.Buffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }
    buffer *Buffer = (buffer*)ResourceManager.Buffers.GetResource(Handle);
    Buffer->ApiData = std::make_shared<d3d12BufferData>();
    std::shared_ptr<d3d12BufferData> D12BufferData = std::static_pointer_cast<d3d12BufferData>(Buffer->ApiData);
    
    Buffer->Init(Size, Usage, MemoryUsage);
    return Handle;
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

bufferHandle context::CreateVertexBuffer(f32 *Values, sz ByteSize, sz Stride, const std::vector<vertexInputAttribute> &Attributes)
{
    GET_CONTEXT(D12Data, this);
    bufferHandle Handle = ResourceManager.Buffers.ObtainResource();
    if(Handle == InvalidHandle)
    {
        return Handle;
    }

    //Create vertex buffer
    buffer *Buffer = (buffer*)ResourceManager.Buffers.GetResource(Handle);
    Buffer->Init(ByteSize, bufferUsage::VertexBuffer, memoryUsage::GpuOnly);
    Buffer->Name = "";
    std::shared_ptr<d3d12BufferData> D12BufferData = std::static_pointer_cast<d3d12BufferData>(Buffer->ApiData);    

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
    SubmitCommandBufferImmediate(D12Data->ImmediateCommandBuffer.get());
    D12Data->StageBuffer.Reset(); 

    // Initialize the vertex buffer view.
    D12BufferData->BufferView.BufferLocation = D12BufferData->Handle->GetGPUVirtualAddress();
    D12BufferData->BufferView.StrideInBytes = Stride;
    D12BufferData->BufferView.SizeInBytes = ByteSize;   
    
    return Handle;
}


void context::SubmitCommandBufferImmediate(commandBuffer *CommandBuffer)
{
    GET_CONTEXT(D12Data, this);
    std::shared_ptr<d3d12CommandBufferData> D12CommandData = std::static_pointer_cast<d3d12CommandBufferData>(CommandBuffer->ApiData);

    // Execute the initialization commands.
	ID3D12CommandList* CommandLists[] = { D12CommandData->CommandList };
	D12Data->CommandQueue->ExecuteCommandLists(1, CommandLists);
}

LPCWSTR ConstCharToLPCWSTR(const char* narrowString) {
    int bufferSize = MultiByteToWideChar(CP_UTF8, 0, narrowString, -1, nullptr, 0);
    wchar_t* wideString = new wchar_t[bufferSize];
    MultiByteToWideChar(CP_UTF8, 0, narrowString, -1, wideString, bufferSize);
    return wideString;
}

std::wstring LPCSTRToWString(LPCSTR str)
{
    // Determine the required length of the wide string
    int wideStrLength = MultiByteToWideChar(CP_ACP, 0, str, -1, nullptr, 0);

    // Allocate memory for the wide string
    wchar_t* wideStrBuffer = new wchar_t[wideStrLength];

    // Convert the narrow string to wide string
    MultiByteToWideChar(CP_ACP, 0, str, -1, wideStrBuffer, wideStrLength);

    // Create a wstring from the wide string
    std::wstring wideStr(wideStrBuffer);

    // Clean up the allocated memory
    delete[] wideStrBuffer;

    return wideStr;
}


ComPtr<IDxcBlob> CompileShader(const shaderStage &Stage, std::vector<D3D12_ROOT_PARAMETER> &OutRootParams, std::unordered_map<u32, u32> &BindingRootParamMapping)
{
    ComPtr<IDxcUtils> Utils;
    ThrowIfFailed(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&Utils)));
    ComPtr<IDxcCompiler3> Compiler;
    ThrowIfFailed(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&Compiler)));
    ComPtr<IDxcIncludeHandler> IncludeHandler;
    ThrowIfFailed(Utils->CreateDefaultIncludeHandler(&IncludeHandler));

    std::vector<LPCWSTR> CompileArgs =
    {
        DXC_ARG_PACK_MATRIX_ROW_MAJOR,
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

    //TODO
    bool Debug=true;
    // Indicate that the shader should be in a debuggable state if in debug mode.
    // Else, set optimization level to 3.
    if(Debug)
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
            ID3D12ShaderReflectionConstantBuffer* shaderReflectionConstantBuffer = shaderReflection->GetConstantBufferByIndex(i);
            D3D12_SHADER_BUFFER_DESC constantBufferDesc{};
            shaderReflectionConstantBuffer->GetDesc(&constantBufferDesc);

            D3D12_ROOT_PARAMETER rootParameter = {};
            rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            rootParameter.Descriptor = {};
            rootParameter.Descriptor.ShaderRegister = shaderInputBindDesc.BindPoint,
            rootParameter.Descriptor.RegisterSpace = shaderInputBindDesc.Space,
            // rootParameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
            
            OutRootParams.push_back(rootParameter);
        }
    } 
    
    return CompiledShaderBlob;
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

        
    //Problem : 
    //There's only one constant buffer that's bound on register 3.
    //There's only 1 root parameter
    {
        D12PipelineData->RootParams.clear();
        D12PipelineData->BindingRootParamMapping.clear();
        D12PipelineData->vertexShader = CompileShader(PipelineCreation.Shaders.Stages[0], D12PipelineData->RootParams, D12PipelineData->BindingRootParamMapping);
        D12PipelineData->pixelShader = CompileShader(PipelineCreation.Shaders.Stages[1], D12PipelineData->RootParams, D12PipelineData->BindingRootParamMapping);

        memset(&D12PipelineData->UsedRootParams[0], 0, d3d12PipelineData::MaxResourceBindings * sizeof(b8));
        for(sz i=0; i<D12PipelineData->RootParams.size(); i++)
        {
            D12PipelineData->UsedRootParams[D12PipelineData->RootParams[i].Descriptor.ShaderRegister]=true;
        }

        D3D12_ROOT_SIGNATURE_DESC rootSigDesc;
        rootSigDesc.NumParameters = D12PipelineData->RootParams.size();
        rootSigDesc.pParameters = D12PipelineData->RootParams.data();
        rootSigDesc.NumStaticSamplers = 0;
        rootSigDesc.pStaticSamplers = nullptr;
        rootSigDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        // create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
        ComPtr<ID3DBlob> serializedRootSig = nullptr;
        ComPtr<ID3DBlob> errorBlob = nullptr;
        HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
            serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());


        if(errorBlob != nullptr)
        {
            ::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
        }
        ThrowIfFailed(hr);

        ThrowIfFailed(D12Data->Device->CreateRootSignature(
            0,
            serializedRootSig->GetBufferPointer(),
            serializedRootSig->GetBufferSize(),
            IID_PPV_ARGS(&D12PipelineData->RootSignature)));
    }

    // Create the pipeline state, which includes compiling and loading shaders.
    {   
        // Define the vertex input layout.
        //TODO: Add ability to have multiple vertex streams
        std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescriptors(PipelineCreation.VertexInput.NumVertexAttributes);        
        u32 Offset=0;
        for(sz i=0; i<PipelineCreation.VertexInput.NumVertexAttributes; i++)
        {
            InputElementDescriptors[i] = 
            {
                SemanticFromAttrib(PipelineCreation.VertexInput.VertexAttributes[i].Format),
                PipelineCreation.VertexInput.VertexAttributes[i].SemanticIndex,
                AttribFormatToNative(PipelineCreation.VertexInput.VertexAttributes[i].Format),
                0, //??????????
                PipelineCreation.VertexInput.VertexAttributes[i].Offset,
                //VertexInputRateToNative(PipelineCreation.VertexInput.VertexStreams[i].InputRate), 
                D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
                0 
            };
            Offset += PipelineCreation.VertexInput.VertexStreams[i].Stride;
        }

        //TODO: Add all pipelineCreation elements in here
        // Describe and create the graphics pipeline state object (PSO).
        D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
        psoDesc.InputLayout = { InputElementDescriptors.data(), (u32)InputElementDescriptors.size() };
        psoDesc.pRootSignature = D12PipelineData->RootSignature.Get();

        psoDesc.VS.BytecodeLength = D12PipelineData->vertexShader->GetBufferSize();
        psoDesc.VS.pShaderBytecode = D12PipelineData->vertexShader->GetBufferPointer();
        psoDesc.PS.BytecodeLength = D12PipelineData->pixelShader->GetBufferSize();
        psoDesc.PS.pShaderBytecode = D12PipelineData->pixelShader->GetBufferPointer();

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

void context::OnResize(u32 NewWidth, u32 NewHeight)
{
    //TODO:
    //Does it just work ? idk
}

void context::Present()
{

}

void context::BindUniformsToPipeline(std::shared_ptr<uniformGroup> Uniforms, pipelineHandle PipelineHandle, u32 Binding){
    GET_CONTEXT(VkData, this);
    pipeline *Pipeline = (pipeline*)ResourceManager.Pipelines.GetResource(PipelineHandle);
    std::shared_ptr<d3d12PipelineData> D12Pipeline = std::static_pointer_cast<d3d12PipelineData>(Pipeline->ApiData);
    std::shared_ptr<d3d12UniformData> D12UniformData = std::static_pointer_cast<d3d12UniformData>(Uniforms->ApiData);
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

void context::DestroyPipeline(pipelineHandle PipelineHandle)
{

}
void context::DestroyBuffer(bufferHandle BufferHandle)
{

}
void context::DestroySwapchain()
{

}

void context::WaitIdle()
{
    GET_CONTEXT(D12Data, this);
    D12Data->VirtualFrames.WaitForPreviousFrame();
    CloseHandle(D12Data->VirtualFrames.FenceEvent);
}

void context::Cleanup()
{

}

D3D12_CPU_DESCRIPTOR_HANDLE d3d12Data::GetCPUDescriptorAt(sz Index)
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(CommonDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), Index, DescriptorSize);
}

D3D12_GPU_DESCRIPTOR_HANDLE d3d12Data::GetGPUDescriptorAt(sz Index)
{
    return CD3DX12_GPU_DESCRIPTOR_HANDLE(CommonDescriptorHeap->GetGPUDescriptorHandleForHeapStart(), Index, DescriptorSize);
}

}