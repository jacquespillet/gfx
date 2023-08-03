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


void d3d11Pipeline::Create(const pipelineCreation &PipelineCreation)
{
    GET_CONTEXT(D11Data, context::Get());

    if(!PipelineCreation.IsCompute)
    {

        //Vertex shader
        ID3DBlob* VSBlob;
        ID3DBlob* pErrorBlob = nullptr;
        
        D3D_SHADER_MACRO defines[] = {
            { "D3D11", "3" }, 
            { "D3D12", "2" }, 
            { "API",   "3" }, 
            nullptr
        };
        HRESULT hr = D3DCompileFromFile(ConstCharToLPCWSTR(PipelineCreation.Shaders.Stages[0].FileName), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", 0, 0, &VSBlob, nullptr);
        if (FAILED(hr))
        {
            if (pErrorBlob)
            {
                char *Error = (char*)pErrorBlob->GetBufferPointer();
                OutputDebugStringA(static_cast<char*>(pErrorBlob->GetBufferPointer()));
                pErrorBlob->Release();
                assert(false);
            }
        }
        D11Data->Device->CreateVertexShader(VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), nullptr, &VertexShader);

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
        
        D11Data->Device->CreateInputLayout(InputElementDescriptors.data(), InputElementDescriptors.size(), VSBlob->GetBufferPointer(), VSBlob->GetBufferSize(), &InputLayout);

        //Pixel shader
        ID3DBlob* PSBlob;
        hr = D3DCompileFromFile(ConstCharToLPCWSTR(PipelineCreation.Shaders.Stages[0].FileName), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", 0, 0, &PSBlob, nullptr);
        if (FAILED(hr))
        {
            if (pErrorBlob)
            {
                char *Error = (char*)pErrorBlob->GetBufferPointer();
                OutputDebugStringA(static_cast<char*>(pErrorBlob->GetBufferPointer()));
                pErrorBlob->Release();
            }
        }
        D11Data->Device->CreatePixelShader(PSBlob->GetBufferPointer(), PSBlob->GetBufferSize(), nullptr, &PixelShader);

        //TODO
        
        //Rasterizer
        D3D11_RASTERIZER_DESC1 rasterizerDesc = {};
        rasterizerDesc.FillMode = D3D11_FILL_SOLID;
        rasterizerDesc.CullMode = D3D11_CULL_NONE;
        D11Data->Device->CreateRasterizerState1(&rasterizerDesc, &RasterizerState);    

        //Sampler
        D3D11_SAMPLER_DESC samplerDesc = {};
        samplerDesc.Filter         = D3D11_FILTER_MIN_MAG_MIP_POINT;
        samplerDesc.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        D11Data->Device->CreateSamplerState(&samplerDesc, &SamplerState);

        //Depth stencil
        D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
        depthStencilDesc.DepthEnable    = TRUE;
        depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
        depthStencilDesc.DepthFunc      = D3D11_COMPARISON_LESS;
        D11Data->Device->CreateDepthStencilState(&depthStencilDesc, &DepthStencilState);

        //Blend state

        //
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

        hr = D11Data->Device->CreateComputeShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &ComputeShader);
        pBlob->Release();    
    }
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