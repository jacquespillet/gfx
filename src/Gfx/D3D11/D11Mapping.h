#pragma once
#include "../Include/Pipeline.h"
#include <d3d11_1.h>

namespace gfx
{
const char *SemanticFromAttrib(vertexComponentFormat::values Format);
DXGI_FORMAT AttribFormatToNative(vertexComponentFormat::values Format);
D3D11_INPUT_CLASSIFICATION  VertexInputRateToNative(vertexInputRate Rate);
DXGI_FORMAT IndexTypeToNative(indexType Type);
DXGI_FORMAT FormatToNative(format Format);

D3D11_CULL_MODE CullModeToNative(cullMode::bits Mode);
b8 FrontFaceToNative(frontFace Face);
D3D11_FILL_MODE FillModeToNative(fillMode Mode);
D3D11_DEPTH_WRITE_MASK DepthWriteToNative(u8 DepthWriteEnabled);
D3D11_COMPARISON_FUNC DepthFuncToNative(compareOperation Operation);
D3D11_BLEND BlendFactorToNative(blendFactor Factor);
D3D11_BLEND BlendFactorAlphaToNative(blendFactor Factor);
D3D11_BLEND_OP BlendOperationToNative(blendOperation Operation);
u8 BlendWriteMaskToNative(colorWriteEnabled::mask Mask);

}