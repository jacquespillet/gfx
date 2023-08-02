#include "D11Mapping.h"

namespace gfx
{

const char *SemanticFromAttrib(vertexComponentFormat::values Format)
{
    // Float, Float2, Float3, Float4, Mat4, Byte, Byte4N, UByte, UByte4N, Short2, Short2N, Short4, Short4N, Uint, Uint2, Uint4, Count
    switch (Format)
    {
    case vertexComponentFormat::Float2:
        return AllocateCString("TEXCOORD");
        break;
    case vertexComponentFormat::Float3:
    case vertexComponentFormat::Float4:
    case vertexComponentFormat::Mat4:
    case vertexComponentFormat::Byte4N:
    case vertexComponentFormat::UByte4N:
    case vertexComponentFormat::Short2:
    case vertexComponentFormat::Short2N:
    case vertexComponentFormat::Short4:
    case vertexComponentFormat::Short4N:
    case vertexComponentFormat::Uint2:
    case vertexComponentFormat::Uint4:
        return AllocateCString("POSITION");
        break;
    case vertexComponentFormat::Float:
        return AllocateCString("BLENDWEIGHT");
        break;
    case vertexComponentFormat::Uint:
    case vertexComponentFormat::Byte:
    case vertexComponentFormat::UByte:
        return AllocateCString("BLENDINDICES");
        break;

    
    default:
        break;
    }

    return "";
}

 
static DXGI_FORMAT VertexAttribFormatTable[] = 
{
    DXGI_FORMAT_R32_FLOAT,
    DXGI_FORMAT_R32G32_FLOAT,
    DXGI_FORMAT_R32G32B32_FLOAT,
    DXGI_FORMAT_R32G32B32A32_FLOAT,
    DXGI_FORMAT_R32G32B32A32_FLOAT, //MAT4 : How to do ??
    DXGI_FORMAT_R8_SINT,
    DXGI_FORMAT_R8G8B8A8_SNORM,
    DXGI_FORMAT_R8_UNORM,
    DXGI_FORMAT_R8G8B8A8_UNORM,
    DXGI_FORMAT_R16G16_SINT,
    DXGI_FORMAT_R16G16_SNORM,
    DXGI_FORMAT_R16G16B16A16_SINT,
    DXGI_FORMAT_R16G16B16A16_SNORM,
    DXGI_FORMAT_R32_UINT,
    DXGI_FORMAT_R32G32_UINT,
    DXGI_FORMAT_R32G32B32A32_UINT
};


DXGI_FORMAT AttribFormatToNative(vertexComponentFormat::values Format)
{
    return VertexAttribFormatTable[(u32)Format];
}


D3D11_INPUT_CLASSIFICATION  VertexInputRateToNative(vertexInputRate Rate)
{
    if(Rate == vertexInputRate::PerVertex)
    {
        return D3D11_INPUT_PER_VERTEX_DATA;   
    }
    else
    {
        return D3D11_INPUT_PER_INSTANCE_DATA;   
    }
}   

DXGI_FORMAT IndexTypeToNative(indexType Type)
{
    return (Type == indexType::Uint16) ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
}


}