#pragma once
#include "../Include/Pipeline.h"
#include <d3d11_1.h>

namespace gfx
{
const char *SemanticFromAttrib(vertexComponentFormat::values Format);
DXGI_FORMAT AttribFormatToNative(vertexComponentFormat::values Format);
D3D11_INPUT_CLASSIFICATION  VertexInputRateToNative(vertexInputRate Rate);
DXGI_FORMAT IndexTypeToNative(indexType Type);

}