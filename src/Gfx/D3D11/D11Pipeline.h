#pragma once

#include <d3d11_1.h>

#include <wrl.h>
using namespace Microsoft::WRL;

namespace gfx
{
struct pipelineCreation;
struct d3d11Pipeline
{
    ComPtr<ID3D11InputLayout> InputLayout = nullptr;
    ComPtr<ID3D11VertexShader> VertexShader = nullptr;
    ComPtr<ID3D11PixelShader> PixelShader = nullptr;
    ComPtr<ID3D11ComputeShader> ComputeShader = nullptr;
    ComPtr<ID3D11RasterizerState1> RasterizerState = nullptr;
    ComPtr<ID3D11SamplerState> SamplerState = nullptr;
    ComPtr<ID3D11DepthStencilState> DepthStencilState = nullptr;
    ComPtr<ID3D11BlendState> BlendState = nullptr;


    void Create(const pipelineCreation &PipelineCreation);
    void DestroyD11Resources();
};
}