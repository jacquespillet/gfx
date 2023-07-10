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
}