#pragma once

#include <d3d11_1.h>
namespace gfx
{
struct pipelineCreation;
struct d3d11Pipeline
{
    ID3D11InputLayout* InputLayout;
    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11ComputeShader* ComputeShader;

    ID3D11RasterizerState1* RasterizerState;
    ID3D11SamplerState* SamplerState;
    ID3D11DepthStencilState* DepthStencilState;
    void Create(const pipelineCreation &PipelineCreation);
};
}