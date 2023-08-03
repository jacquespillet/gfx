#pragma once

#include <d3d11_1.h>
namespace gfx
{
struct pipelineCreation;
struct d3d11Pipeline
{
    ID3D11InputLayout* InputLayout = nullptr;
    ID3D11VertexShader* VertexShader = nullptr;
    ID3D11PixelShader* PixelShader = nullptr;
    ID3D11ComputeShader* ComputeShader = nullptr;

    ID3D11RasterizerState1* RasterizerState = nullptr;
    ID3D11SamplerState* SamplerState = nullptr;
    ID3D11DepthStencilState* DepthStencilState = nullptr;
    void Create(const pipelineCreation &PipelineCreation);
    void DestroyD11Resources();
};
}