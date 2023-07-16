#pragma once
#include "../Include/Types.h"
#include "../Include/Pipeline.h"

#include <dxgi1_6.h>
#include <d3d12.h>

namespace gfx
{

const char *SemanticFromAttrib(vertexComponentFormat::values Format);
DXGI_FORMAT AttribFormatToNative(vertexComponentFormat::values Format);
D3D12_INPUT_CLASSIFICATION  VertexInputRateToNative(vertexInputRate::values Rate);
DXGI_FORMAT FormatToNative(format Format);


D3D12_RESOURCE_STATES ImageUsageToResourceState(imageUsage::bits Usage);

D3D12_COMPARISON_FUNC DepthFuncToNative(compareOperation Operation);
D3D12_DEPTH_WRITE_MASK DepthWriteToNative(u8 DepthWriteEnabled);
D3D12_DEPTH_STENCILOP_DESC StencilStateToNative(stencilOperationState Operation);
D3D12_BLEND BlendFactorToNative(blendFactor Factor);
D3D12_BLEND_OP BlendOperationToNative(blendOperation Operation);
u8 BlendWriteMaskToNative(colorWriteEnabled::mask Mask);
D3D12_CULL_MODE CullModeToNative(cullMode::bits Mode);
b8 FrontFaceToNative(frontFace Face);
D3D12_FILL_MODE FillModeToNative(fillMode Mode);

DXGI_FORMAT IndexTypeToNative(indexType Type);
}