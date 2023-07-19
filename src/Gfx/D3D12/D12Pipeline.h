#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <dxc/dxcapi.h>

using namespace Microsoft::WRL;

namespace gfx
{
struct d3d12PipelineData
{
    ComPtr<ID3D12RootSignature> RootSignature;
    ComPtr<ID3D12PipelineState> PipelineState;

    ComPtr<IDxcBlob> vertexShader;
    ComPtr<IDxcBlob> pixelShader;
    ComPtr<IDxcBlob> computeShader;
    
    std::vector<D3D12_ROOT_PARAMETER> RootParams;
    b8 UsedRootParams[d12Constants::MaxResourceBindings]; 
    
    std::unordered_map<u32, u32> BindingRootParamMapping;

    b8 IsCompute=false;
};

}