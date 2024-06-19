#include "D11Pipeline.h"
#include "../Include/Pipeline.h"
#include "../Include/Context.h"

#include "D11Context.h"
#include "D11Common.h"
#include "D11Mapping.h"

#include <d3d11_1.h>
#include <d3dcompiler.h>

namespace gfx
{
std::vector<ComPtr<ID3D11SamplerState>> d3d11Pipeline::Samplers = std::vector<ComPtr<ID3D11SamplerState>>();

D3D11_BLEND_DESC CreateDefaultBlendDesc() {
    D3D11_BLEND_DESC BlendDesc = {};

    BlendDesc.AlphaToCoverageEnable = FALSE;
    BlendDesc.IndependentBlendEnable = FALSE;

    for (UINT i = 0; i < 8; ++i) {
        BlendDesc.RenderTarget[i].BlendEnable = FALSE;
        BlendDesc.RenderTarget[i].SrcBlend = D3D11_BLEND_ONE;
        BlendDesc.RenderTarget[i].DestBlend = D3D11_BLEND_ZERO;
        BlendDesc.RenderTarget[i].BlendOp = D3D11_BLEND_OP_ADD;
        BlendDesc.RenderTarget[i].SrcBlendAlpha = D3D11_BLEND_ONE;
        BlendDesc.RenderTarget[i].DestBlendAlpha = D3D11_BLEND_ZERO;
        BlendDesc.RenderTarget[i].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        BlendDesc.RenderTarget[i].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    }

    return BlendDesc;
}

void d3d11Pipeline::Create(const pipelineCreation &PipelineCreation)
{
    GET_CONTEXT(D11Data, context::Get());
    renderPass *RenderPass = context::Get()->GetRenderPass(PipelineCreation.RenderPassHandle);

    if(!PipelineCreation.IsCompute)
    {

        //Vertex shadera
        ID3DBlob* VSBlob;
        ID3DBlob* pErrorBlob = nullptr;
        
        D3D_SHADER_MACRO defines[] = {
            { "GL", "0" }, 
            { "VK", "1" }, 
            { "D3D11", "3" }, 
            { "D3D12", "2" }, 
            { "GRAPHICS_API",   "3" }, 
            nullptr
        };
        HRESULT hr = D3DCompileFromFile(ConstCharToLPCWSTR(PipelineCreation.Shaders.Stages[0].FileName), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", 0, 0, &VSBlob, &pErrorBlob);
        if (FAILED(hr) || VSBlob == nullptr)
        {
            if (pErrorBlob)
            {
                char *Error = (char*)pErrorBlob->GetBufferPointer();
                OutputDebugStringA(static_cast<char*>(pErrorBlob->GetBufferPointer()));
                pErrorBlob->Release();
            }
            assert(false);
        }
        D11Data->Device->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), nullptr, VertexShader.GetAddressOf());

        std::vector<D3D11_INPUT_ELEMENT_DESC> InputElementDescriptors(PipelineCreation.VertexInput.NumVertexAttributes);        
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
        
        D11Data->Device->CreateInputLayout(InputElementDescriptors.data(), InputElementDescriptors.size(), VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), InputLayout.GetAddressOf());

        //Pixel shader
        ID3DBlob* PSBlob;
        hr = D3DCompileFromFile(ConstCharToLPCWSTR(PipelineCreation.Shaders.Stages[0].FileName), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", 0, 0, &PSBlob, &pErrorBlob);
        if (FAILED(hr))
        {
            if (pErrorBlob)
            {
                char *Error = (char*)pErrorBlob->GetBufferPointer();
                OutputDebugStringA(static_cast<char*>(pErrorBlob->GetBufferPointer()));
                pErrorBlob->Release();
            }
        }
        D11Data->Device->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), nullptr, PixelShader.GetAddressOf());

        //Rasterizer
        D3D11_RASTERIZER_DESC1 rasterizerDesc = {};
        rasterizerDesc.FillMode = FillModeToNative(PipelineCreation.Rasterization.Fill);
        rasterizerDesc.CullMode = CullModeToNative(PipelineCreation.Rasterization.CullMode);
        rasterizerDesc.FrontCounterClockwise = FrontFaceToNative(PipelineCreation.Rasterization.FrontFace);
        D11Data->Device->CreateRasterizerState1(&rasterizerDesc, RasterizerState.GetAddressOf());    

        //Sampler
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        samplerDesc.MinLOD = 0;
        samplerDesc.MaxLOD = 11;
        D11Data->Device->CreateSamplerState(&samplerDesc, SamplerState.GetAddressOf());

        if(d3d11Pipeline::Samplers.size()==0)
        {
            d3d11Pipeline::Samplers.resize(1);
            D3D11_SAMPLER_DESC SamplerDesc = {};
            SamplerDesc.Filter         = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
            SamplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_BORDER;
            SamplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_BORDER;
            SamplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_BORDER;
            SamplerDesc.BorderColor[0] = 1.0f;
            SamplerDesc.BorderColor[1] = 1.0f;
            SamplerDesc.BorderColor[2] = 1.0f;
            SamplerDesc.BorderColor[3] = 1.0f;
            SamplerDesc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
            D11Data->Device->CreateSamplerState(&SamplerDesc, d3d11Pipeline::Samplers[0].GetAddressOf());
        }

        //Depth stencil
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable    = PipelineCreation.DepthStencil.DepthEnable;
        depthStencilDesc.DepthWriteMask = DepthWriteToNative(PipelineCreation.DepthStencil.DepthWriteEnable);
        depthStencilDesc.DepthFunc      = DepthFuncToNative(PipelineCreation.DepthStencil.DepthComparison);
        D11Data->Device->CreateDepthStencilState(&depthStencilDesc, DepthStencilState.GetAddressOf());

        //Blend state
        D3D11_BLEND_DESC BlendStateDesc = CreateDefaultBlendDesc();
        for(sz i=0; i<PipelineCreation.BlendState.ActiveStates; i++)
        {
            BlendStateDesc.RenderTarget[i].SrcBlend = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[i].SourceColor);
            BlendStateDesc.RenderTarget[i].DestBlend = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[i].DestinationColor);
            if(PipelineCreation.BlendState.BlendStates[i].SeparateBlend)
            {
                BlendStateDesc.RenderTarget[i].SrcBlendAlpha = BlendFactorAlphaToNative(PipelineCreation.BlendState.BlendStates[i].SourceAlpha);
                BlendStateDesc.RenderTarget[i].DestBlendAlpha = BlendFactorAlphaToNative(PipelineCreation.BlendState.BlendStates[i].DestinationAlpha);
                BlendStateDesc.RenderTarget[i].SrcBlend = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[i].SourceColor);
                BlendStateDesc.RenderTarget[i].DestBlend = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[i].DestinationColor);
            }
            else
            {
                BlendStateDesc.RenderTarget[i].SrcBlendAlpha = BlendFactorAlphaToNative(PipelineCreation.BlendState.BlendStates[i].SourceColor);
                BlendStateDesc.RenderTarget[i].DestBlendAlpha = BlendFactorAlphaToNative(PipelineCreation.BlendState.BlendStates[i].DestinationColor);
                BlendStateDesc.RenderTarget[i].SrcBlend = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[i].SourceColor);
                BlendStateDesc.RenderTarget[i].DestBlend = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[i].DestinationColor);
            }

            BlendStateDesc.RenderTarget[i].BlendOp = BlendOperationToNative(PipelineCreation.BlendState.BlendStates[i].ColorOp);
            BlendStateDesc.RenderTarget[i].BlendOpAlpha = BlendOperationToNative(PipelineCreation.BlendState.BlendStates[i].AlphaOp);
            BlendStateDesc.RenderTarget[i].BlendEnable = (b8)PipelineCreation.BlendState.BlendStates[i].BlendEnabled;
            BlendStateDesc.RenderTarget[i].RenderTargetWriteMask = BlendWriteMaskToNative(PipelineCreation.BlendState.BlendStates[i].ColorWriteMask);
        }
        D11Data->Device->CreateBlendState(&BlendStateDesc, BlendState.GetAddressOf());

        for(sz i=0; i<InputElementDescriptors.size(); i++)
        {
            DeallocateMemory((void*)InputElementDescriptors[i].SemanticName);
        }
    }
    else
    {
        ID3DBlob* pBlob = nullptr;
        ID3DBlob* pErrorBlob = nullptr;
        D3D_SHADER_MACRO defines[] = {
            { "D3D11", "3" },
            { "D3D12", "2" },
            { "API",   "3" },
            nullptr
        };
        HRESULT hr = D3DCompileFromFile(ConstCharToLPCWSTR(PipelineCreation.Shaders.Stages[0].FileName), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0",
                                        D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, &pBlob, &pErrorBlob);
        if (FAILED(hr))
        {
            if (pErrorBlob)
            {
                const char* Error = (const char*)pErrorBlob->GetBufferPointer();
                OutputDebugStringA((char*)pErrorBlob->GetBufferPointer());
                pErrorBlob->Release();
                assert(false);
            }
        }

        hr = D11Data->Device->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, ComputeShader.GetAddressOf());
        pBlob->Release();    
    }
        
    for (size_t j = 0; j < PipelineCreation.Shaders.StagesCount; j++)
    {
        DeallocateMemory((void*)PipelineCreation.Shaders.Stages[j].Code);
        DeallocateMemory((void*)PipelineCreation.Shaders.Stages[j].FileName);
    }
    DeallocateMemory((void*)PipelineCreation.Name);
}

void d3d11Pipeline::DestroyD11Resources()
{
    if(InputLayout != nullptr) InputLayout->Release();
    if(VertexShader != nullptr) VertexShader->Release();
    if(PixelShader != nullptr) PixelShader->Release();
    if(ComputeShader != nullptr) ComputeShader->Release();
    if(RasterizerState != nullptr) RasterizerState->Release();
    if(SamplerState != nullptr) SamplerState->Release();
    if(DepthStencilState != nullptr) DepthStencilState->Release();    
}
}