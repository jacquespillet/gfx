#include "D12Pipeline.h"
#include "../Include/RenderPass.h"
#include "../Include/Pipeline.h"
#include "../Include/Context.h"

#include "D12Mapping.h"
#include "D12Common.h"
#include "D12Context.h"

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


namespace gfx
{

void d3d12PipelineData::DestroyD12Resources()
{
    RootSignature.Reset();
    PipelineState.Reset();
    if(vertexShader!=nullptr) vertexShader.Reset();
    if(pixelShader!=nullptr)pixelShader.Reset();
    if(computeShader!=nullptr)computeShader.Reset();

    BindingRootParamMapping.clear();
    RootParams.clear();
    memset(UsedRootParams, 0, sizeof(UsedRootParams));
}

ComPtr<IDxcBlob> CompileShader2(const shaderStage &Stage, std::vector<D3D12_ROOT_PARAMETER> &OutRootParams, std::unordered_map<u32, u32> &BindingRootParamMapping, std::vector<CD3DX12_DESCRIPTOR_RANGE> &DescriptorRanges, b8 *UsedRootParams)
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

    CompileArgs.push_back(L"-DGL=0");
    CompileArgs.push_back(L"-DVK=1");
    CompileArgs.push_back(L"-DD3D11=3");
    CompileArgs.push_back(L"-DD3D12=2");
    CompileArgs.push_back(L"-DGRAPHICS_API=2");

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
        if (shaderInputBindDesc.Type == D3D_SIT_CBUFFER && !UsedRootParams[shaderInputBindDesc.BindPoint])
        {
            BindingRootParamMapping[shaderInputBindDesc.BindPoint] = static_cast<uint32_t>(OutRootParams.size());

            D3D12_ROOT_PARAMETER rootParameter = {};
            rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
            rootParameter.Descriptor = {};
            rootParameter.Descriptor.ShaderRegister = shaderInputBindDesc.BindPoint,
            rootParameter.Descriptor.RegisterSpace = shaderInputBindDesc.Space,
            // rootParameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
            UsedRootParams[shaderInputBindDesc.BindPoint] = true;
            OutRootParams.push_back(rootParameter);
        }
        else if (shaderInputBindDesc.Type == D3D_SIT_UAV_RWSTRUCTURED && !UsedRootParams[shaderInputBindDesc.BindPoint])
        {
            BindingRootParamMapping[shaderInputBindDesc.BindPoint] = static_cast<uint32_t>(OutRootParams.size());

            D3D12_ROOT_PARAMETER rootParameter = {};
            rootParameter.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
            rootParameter.Descriptor = {};
            rootParameter.Descriptor.ShaderRegister = shaderInputBindDesc.BindPoint,
            rootParameter.Descriptor.RegisterSpace = shaderInputBindDesc.Space,
            // rootParameter.Descriptor.Flags = D3D12_ROOT_DESCRIPTOR_FLAG_NONE,
            UsedRootParams[shaderInputBindDesc.BindPoint] = true;
            
            OutRootParams.push_back(rootParameter);
        }
        else if (shaderInputBindDesc.Type == D3D_SIT_TEXTURE && !UsedRootParams[shaderInputBindDesc.BindPoint])
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

            UsedRootParams[shaderInputBindDesc.BindPoint] = true;

            OutRootParams.push_back(RootParameter);  
        }
    } 
    
    return CompiledShaderBlob;
}


void d3d12PipelineData::Create(const pipelineCreation &PipelineCreation)
{
    GET_CONTEXT(D12Data, context::Get());

    renderPass *RenderPass = context::Get()->GetRenderPass(PipelineCreation.RenderPassHandle);

    //Root signature        
    {
        std::vector<CD3DX12_DESCRIPTOR_RANGE> DescriptorRanges;
        DescriptorRanges.reserve(1024);

        memset(&this->UsedRootParams[0], 0, d12Constants::MaxResourceBindings * sizeof(b8));
        this->RootParams.clear();
        this->BindingRootParamMapping.clear();
        for(u32 i=0; i<PipelineCreation.Shaders.StagesCount; i++)
        {
            if(PipelineCreation.Shaders.Stages[i].Stage == shaderStageFlags::Vertex)
                this->vertexShader = CompileShader2(PipelineCreation.Shaders.Stages[i], this->RootParams, this->BindingRootParamMapping, DescriptorRanges, UsedRootParams);
            if(PipelineCreation.Shaders.Stages[i].Stage == shaderStageFlags::Fragment)
                this->pixelShader = CompileShader2(PipelineCreation.Shaders.Stages[i], this->RootParams, this->BindingRootParamMapping, DescriptorRanges, UsedRootParams);
            if(PipelineCreation.Shaders.Stages[i].Stage == shaderStageFlags::Compute)
                this->computeShader = CompileShader2(PipelineCreation.Shaders.Stages[i], this->RootParams, this->BindingRootParamMapping, DescriptorRanges, UsedRootParams);
        }

        // for(sz i=0; i<this->RootParams.size(); i++)
        // {
        //     if(this->RootParams[i].ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
        //     {
        //         this->UsedRootParams[this->RootParams[i].DescriptorTable.pDescriptorRanges[0].BaseShaderRegister]=true;
        //     }
        //     else
        //     {
        //         this->UsedRootParams[this->RootParams[i].Descriptor.ShaderRegister]=true;
        //     }
        // }

        const CD3DX12_STATIC_SAMPLER_DESC pointWrap( 0, // shaderRegister    
                                            D3D12_FILTER_MIN_MAG_MIP_POINT, // filter    
                                            D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressU    
                                            D3D12_TEXTURE_ADDRESS_MODE_WRAP, // addressV    
                                            D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW  

        D3D12_ROOT_SIGNATURE_DESC rootSigDesc;
        rootSigDesc.NumParameters = (u32)this->RootParams.size();
        rootSigDesc.pParameters = this->RootParams.data();
        rootSigDesc.NumStaticSamplers = 1;
        rootSigDesc.pStaticSamplers = &pointWrap;
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
            IID_PPV_ARGS(&this->RootSignature)));
    }

    {   

        if(!PipelineCreation.IsCompute)
        {
            //Input 
            std::vector<D3D12_INPUT_ELEMENT_DESC> InputElementDescriptors(PipelineCreation.VertexInput.NumVertexAttributes);        
            u32 Offset=0;
            for(sz i=0; i<PipelineCreation.VertexInput.NumVertexAttributes; i++)
            {
                InputElementDescriptors[i] =   
                {
                    SemanticFromAttrib(PipelineCreation.VertexInput.VertexAttributes[i].Format),
                    PipelineCreation.VertexInput.VertexAttributes[i].SemanticIndex,
                    AttribFormatToNative(PipelineCreation.VertexInput.VertexAttributes[i].Format),
                    PipelineCreation.VertexInput.VertexAttributes[i].Binding,
                    PipelineCreation.VertexInput.VertexAttributes[i].Offset,
                    VertexInputRateToNative(PipelineCreation.VertexInput.VertexStreams[PipelineCreation.VertexInput.VertexAttributes[i].Binding].InputRate), 
                    (PipelineCreation.VertexInput.VertexStreams[PipelineCreation.VertexInput.VertexAttributes[i].Binding].InputRate == vertexInputRate::PerVertex) ? 0u : 1u
                };
                Offset += PipelineCreation.VertexInput.VertexStreams[PipelineCreation.VertexInput.VertexAttributes[i].Binding].Stride;
            }

            // Describe and create the graphics pipeline state object (PSO).
            D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.InputLayout = { InputElementDescriptors.data(), (u32)InputElementDescriptors.size() };
            psoDesc.pRootSignature = this->RootSignature.Get();

            psoDesc.VS.BytecodeLength = this->vertexShader->GetBufferSize();
            psoDesc.VS.pShaderBytecode = this->vertexShader->GetBufferPointer();
            psoDesc.PS.BytecodeLength = this->pixelShader->GetBufferSize();
            psoDesc.PS.pShaderBytecode = this->pixelShader->GetBufferPointer();

            psoDesc.DepthStencilState.DepthFunc = DepthFuncToNative(PipelineCreation.DepthStencil.DepthComparison);
            psoDesc.DepthStencilState.DepthEnable = (b8)(PipelineCreation.DepthStencil.DepthEnable);
            psoDesc.DepthStencilState.StencilEnable = (b8)(PipelineCreation.DepthStencil.StencilEnable);
            psoDesc.DepthStencilState.DepthWriteMask = DepthWriteToNative(PipelineCreation.DepthStencil.DepthWriteEnable);
            psoDesc.DepthStencilState.FrontFace = StencilStateToNative(PipelineCreation.DepthStencil.Front);
            psoDesc.DepthStencilState.BackFace = StencilStateToNative(PipelineCreation.DepthStencil.Back);
            
            psoDesc.NumRenderTargets = RenderPass->Output.NumColorFormats;
            psoDesc.DSVFormat = FormatToNative(RenderPass->Output.DepthStencilFormat);

            psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
            for(sz i=0; i<psoDesc.NumRenderTargets; i++)
            {
                psoDesc.BlendState.RenderTarget[i].SrcBlend = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[i].SourceColor);
                psoDesc.BlendState.RenderTarget[i].DestBlend = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[i].DestinationColor);
                if(PipelineCreation.BlendState.BlendStates[i].SeparateBlend)
                {
                    psoDesc.BlendState.RenderTarget[i].SrcBlendAlpha = BlendFactorAlphaToNative(PipelineCreation.BlendState.BlendStates[i].SourceAlpha);
                    psoDesc.BlendState.RenderTarget[i].DestBlendAlpha = BlendFactorAlphaToNative(PipelineCreation.BlendState.BlendStates[i].DestinationAlpha);
                    psoDesc.BlendState.RenderTarget[i].SrcBlend = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[i].SourceColor);
                    psoDesc.BlendState.RenderTarget[i].DestBlend = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[i].DestinationColor);
                }
                else
                {
                    psoDesc.BlendState.RenderTarget[i].SrcBlendAlpha = BlendFactorAlphaToNative(PipelineCreation.BlendState.BlendStates[i].SourceColor);
                    psoDesc.BlendState.RenderTarget[i].DestBlendAlpha = BlendFactorAlphaToNative(PipelineCreation.BlendState.BlendStates[i].DestinationColor);
                    psoDesc.BlendState.RenderTarget[i].SrcBlend = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[i].SourceColor);
                    psoDesc.BlendState.RenderTarget[i].DestBlend = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[i].DestinationColor);
                }

                psoDesc.BlendState.RenderTarget[i].BlendOp = BlendOperationToNative(PipelineCreation.BlendState.BlendStates[i].ColorOp);
                psoDesc.BlendState.RenderTarget[i].BlendOpAlpha = BlendOperationToNative(PipelineCreation.BlendState.BlendStates[i].AlphaOp);
                psoDesc.BlendState.RenderTarget[i].BlendEnable = (b8)PipelineCreation.BlendState.BlendStates[i].BlendEnabled;
                psoDesc.BlendState.RenderTarget[i].RenderTargetWriteMask = BlendWriteMaskToNative(PipelineCreation.BlendState.BlendStates[i].ColorWriteMask);
                
                psoDesc.RTVFormats[i] = FormatToNative(RenderPass->Output.ColorFormats[i]);
            }
            
            psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
            psoDesc.RasterizerState.CullMode = CullModeToNative(PipelineCreation.Rasterization.CullMode);
            psoDesc.RasterizerState.FrontCounterClockwise = FrontFaceToNative(PipelineCreation.Rasterization.FrontFace);
            psoDesc.RasterizerState.FillMode = FillModeToNative(PipelineCreation.Rasterization.Fill);
            psoDesc.RasterizerState.MultisampleEnable = RenderPass->Output.SampleCount > 1 ? TRUE : FALSE;
            psoDesc.SampleDesc.Count = RenderPass->Output.SampleCount;
            psoDesc.SampleDesc.Quality = 0;

            psoDesc.SampleMask = UINT_MAX;
            psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
            ThrowIfFailed(D12Data->Device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&this->PipelineState)));
            
            this->IsCompute = false;
            for(sz i=0; i<InputElementDescriptors.size(); i++)
            {
                DeallocateMemory((void*)InputElementDescriptors[i].SemanticName);
            }
        }
        else
        {
            this->IsCompute = true;
            // Describe the compute pipeline state
            D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
            psoDesc.pRootSignature = this->RootSignature.Get(); // The root signature for the compute shader
            psoDesc.CS.BytecodeLength = this->computeShader->GetBufferSize();
            psoDesc.CS.pShaderBytecode = this->computeShader->GetBufferPointer();
            
            ThrowIfFailed(D12Data->Device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&this->PipelineState)));
        }

    }    
    
    for (size_t j = 0; j < PipelineCreation.Shaders.StagesCount; j++)
    {
        DeallocateMemory((void*)PipelineCreation.Shaders.Stages[j].Code);
        DeallocateMemory((void*)PipelineCreation.Shaders.Stages[j].FileName);
    }
    DeallocateMemory((void*)PipelineCreation.Name);
}

}