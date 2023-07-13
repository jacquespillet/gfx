#include "D12Mapping.h"
#include <dxgi.h>
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

// Float, Float2, Float3, Float4, Mat4, Byte, Byte4N, UByte, UByte4N, Short2, Short2N, Short4, Short4N, Uint, Uint2, Uint4, Count
   
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

D3D12_INPUT_CLASSIFICATION  VertexInputRateToNative(vertexInputRate::values Rate)
{
    if(Rate == vertexInputRate::PerVertex)
    {
        return D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;   
    }
    else
    {
        return D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;   
    }
}   


static DXGI_FORMAT FormatTable[] = 
{
    DXGI_FORMAT_UNKNOWN,//UNDEFINED = 0,
    DXGI_FORMAT_UNKNOWN,//R4G4_UNORM_PACK_8,
    DXGI_FORMAT_UNKNOWN,//R4G4B4A4_UNORM_PACK_16,
    DXGI_FORMAT_UNKNOWN,//B4G4R4A4_UNORM_PACK_16,
    DXGI_FORMAT_UNKNOWN,//R5G6B5_UNORM_PACK_16,
    DXGI_FORMAT_UNKNOWN,//B5G6R5_UNORM_PACK_16,
    DXGI_FORMAT_UNKNOWN,//R5G5B5A1_UNORM_PACK_16,
    DXGI_FORMAT_UNKNOWN,//B5G5R5A1_UNORM_PACK_16,
    DXGI_FORMAT_UNKNOWN,//A1R5G5B5_UNORM_PACK_16,
    DXGI_FORMAT_R8_UNORM,//R8_UNORM,
    DXGI_FORMAT_R8_SNORM,//R8_SNORM,
    DXGI_FORMAT_R8_UNORM,//R8_USCALED,
    DXGI_FORMAT_R8_UNORM,//R8_SSCALED,
    DXGI_FORMAT_R8_UINT,//R8_UINT,
    DXGI_FORMAT_R8_SINT,//R8_SINT,
    DXGI_FORMAT_UNKNOWN,//R8_SRGB,
    DXGI_FORMAT_R8G8_UNORM,//R8G8_UNORM,
    DXGI_FORMAT_R8G8_SNORM,//R8G8_SNORM,
    DXGI_FORMAT_R8G8_UNORM,//R8G8_USCALED,
    DXGI_FORMAT_R8G8_UNORM,//R8G8_SSCALED,
    DXGI_FORMAT_R8G8_UINT,//R8G8_UINT,
    DXGI_FORMAT_R8G8_SINT,//R8G8_SINT,
    DXGI_FORMAT_UNKNOWN,//R8G8_SRGB,
    DXGI_FORMAT_R8G8B8A8_UNORM,//R8G8B8_UNORM,
    DXGI_FORMAT_R8G8B8A8_SNORM,//R8G8B8_SNORM,
    DXGI_FORMAT_R8G8B8A8_UNORM,//R8G8B8_USCALED,
    DXGI_FORMAT_R8G8B8A8_UNORM,//R8G8B8_SSCALED,
    DXGI_FORMAT_R8G8B8A8_UINT,//R8G8B8_UINT,
    DXGI_FORMAT_R8G8B8A8_SINT,//R8G8B8_SINT,
    DXGI_FORMAT_UNKNOWN,//R8G8B8_SRGB,
    DXGI_FORMAT_UNKNOWN,//B8G8R8_UNORM,
    DXGI_FORMAT_UNKNOWN,//B8G8R8_SNORM,
    DXGI_FORMAT_B8G8R8A8_UNORM,//B8G8R8_USCALED,
    DXGI_FORMAT_B8G8R8A8_UNORM,//B8G8R8_SSCALED,
    DXGI_FORMAT_UNKNOWN,//B8G8R8_UINT,
    DXGI_FORMAT_UNKNOWN,//B8G8R8_SINT,
    DXGI_FORMAT_UNKNOWN,//B8G8R8_SRGB,
    DXGI_FORMAT_R8G8B8A8_UNORM,//R8G8B8A8_UNORM,
    DXGI_FORMAT_R8G8B8A8_SNORM,//R8G8B8A8_SNORM,
    DXGI_FORMAT_R8G8B8A8_UNORM,//R8G8B8A8_USCALED,
    DXGI_FORMAT_R8G8B8A8_UNORM,//R8G8B8A8_SSCALED,
    DXGI_FORMAT_R8G8B8A8_UINT,//R8G8B8A8_UINT,
    DXGI_FORMAT_R8G8B8A8_SINT,//R8G8B8A8_SINT,
    DXGI_FORMAT_UNKNOWN,//R8G8B8A8_SRGB,
    DXGI_FORMAT_B8G8R8A8_UNORM,//B8G8R8A8_UNORM,
    DXGI_FORMAT_UNKNOWN,//B8G8R8A8_SNORM,
    DXGI_FORMAT_B8G8R8A8_UNORM,//B8G8R8A8_USCALED,
    DXGI_FORMAT_B8G8R8A8_UNORM,//B8G8R8A8_SSCALED,
    DXGI_FORMAT_UNKNOWN,//B8G8R8A8_UINT,
    DXGI_FORMAT_UNKNOWN,//B8G8R8A8_SINT,
    DXGI_FORMAT_UNKNOWN,//B8G8R8A8_SRGB,
    DXGI_FORMAT_UNKNOWN,//A8B8G8R8_UNORM_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A8B8G8R8_SNORM_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A8B8G8R8_USCALED_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A8B8G8R8_SSCALED_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A8B8G8R8_UINT_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A8B8G8R8_SINT_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A8B8G8R8_SRGB_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A2R10G10B10_UNORM_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A2R10G10B10_SNORM_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A2R10G10B10_USCALED_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A2R10G10B10_SSCALED_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A2R10G10B10_UINT_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A2R10G10B10_SINT_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A2B10G10R10_UNORM_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A2B10G10R10_SNORM_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A2B10G10R10_USCALED_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A2B10G10R10_SSCALED_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A2B10G10R10_UINT_PACK_32,
    DXGI_FORMAT_UNKNOWN,//A2B10G10R10_SINT_PACK_32,
    DXGI_FORMAT_R16_UNORM,//R16_UNORM,
    DXGI_FORMAT_R16_SNORM,//R16_SNORM,
    DXGI_FORMAT_R16_UNORM,//R16_USCALED,
    DXGI_FORMAT_R16_UNORM,//R16_SSCALED,
    DXGI_FORMAT_R16_UINT,//R16_UINT,
    DXGI_FORMAT_R16_SINT,//R16_SINT,
    DXGI_FORMAT_R16_FLOAT,//R16_SFLOAT,
    DXGI_FORMAT_R16G16_UNORM,//R16G16_UNORM,
    DXGI_FORMAT_R16G16_SNORM,//R16G16_SNORM,
    DXGI_FORMAT_R16G16_TYPELESS,//R16G16_USCALED,
    DXGI_FORMAT_R16G16_TYPELESS,//R16G16_SSCALED,
    DXGI_FORMAT_R16G16_UNORM,//R16G16_UINT,
    DXGI_FORMAT_R16G16_SINT,//R16G16_SINT,
    DXGI_FORMAT_R16G16_FLOAT,//R16G16_SFLOAT,
    DXGI_FORMAT_UNKNOWN,//R16G16B16_UNORM,
    DXGI_FORMAT_UNKNOWN,//R16G16B16_SNORM,
    DXGI_FORMAT_UNKNOWN,//R16G16B16_USCALED,
    DXGI_FORMAT_UNKNOWN,//R16G16B16_SSCALED,
    DXGI_FORMAT_UNKNOWN,//R16G16B16_UINT,
    DXGI_FORMAT_UNKNOWN,//R16G16B16_SINT,
    DXGI_FORMAT_UNKNOWN,//R16G16B16_SFLOAT,
    DXGI_FORMAT_R16G16B16A16_UNORM,//R16G16B16A16_UNORM,
    DXGI_FORMAT_R16G16B16A16_SNORM,//R16G16B16A16_SNORM,
    DXGI_FORMAT_R16G16B16A16_TYPELESS,//R16G16B16A16_USCALED,
    DXGI_FORMAT_R16G16B16A16_TYPELESS,//R16G16B16A16_SSCALED,
    DXGI_FORMAT_R16G16B16A16_UINT,//R16G16B16A16_UINT,
    DXGI_FORMAT_R16G16B16A16_SINT,//R16G16B16A16_SINT,
    DXGI_FORMAT_R16G16B16A16_FLOAT,//R16G16B16A16_SFLOAT,
    DXGI_FORMAT_R32_UINT,//R32_UINT,
    DXGI_FORMAT_R32_SINT,//R32_SINT,
    DXGI_FORMAT_R32_FLOAT,//R32_SFLOAT,
    DXGI_FORMAT_R32G32_UINT,//R32G32_UINT,
    DXGI_FORMAT_R32G32_SINT,//R32G32_SINT,
    DXGI_FORMAT_R32G32_FLOAT,//R32G32_SFLOAT,
    DXGI_FORMAT_R32G32B32_UINT,//R32G32B32_UINT,
    DXGI_FORMAT_R32G32B32_SINT,//R32G32B32_SINT,
    DXGI_FORMAT_R32G32B32_FLOAT,//R32G32B32_SFLOAT,
    DXGI_FORMAT_R32G32B32A32_UINT,//R32G32B32A32_UINT,
    DXGI_FORMAT_R32G32B32A32_SINT,//R32G32B32A32_SINT,
    DXGI_FORMAT_R32G32B32A32_FLOAT,//R32G32B32A32_SFLOAT,
    DXGI_FORMAT_UNKNOWN,//R64_UINT,
    DXGI_FORMAT_UNKNOWN,//R64_SINT,
    DXGI_FORMAT_UNKNOWN,//R64_SFLOAT,
    DXGI_FORMAT_UNKNOWN,//R64G64_UINT,
    DXGI_FORMAT_UNKNOWN,//R64G64_SINT,
    DXGI_FORMAT_UNKNOWN,//R64G64_SFLOAT,
    DXGI_FORMAT_UNKNOWN,//R64G64B64_UINT,
    DXGI_FORMAT_UNKNOWN,//R64G64B64_SINT,
    DXGI_FORMAT_UNKNOWN,//R64G64B64_SFLOAT,
    DXGI_FORMAT_UNKNOWN,//R64G64B64A64_UINT,
    DXGI_FORMAT_UNKNOWN,//R64G64B64A64_SINT,
    DXGI_FORMAT_UNKNOWN,//R64G64B64A64_SFLOAT,
    DXGI_FORMAT_UNKNOWN,//B10G11R11_UFLOAT_PACK_32,
    DXGI_FORMAT_UNKNOWN,//E5B9G9R9_UFLOAT_PACK_32,
    DXGI_FORMAT_D16_UNORM,//D16_UNORM,
    DXGI_FORMAT_UNKNOWN,//X8D24_UNORM_PACK_32,
    DXGI_FORMAT_D32_FLOAT,//D32_SFLOAT,
    DXGI_FORMAT_UNKNOWN,//S8_UINT,
    DXGI_FORMAT_UNKNOWN,//D16_UNORM_S8_UINT,
    DXGI_FORMAT_D24_UNORM_S8_UINT,//D24_UNORM_S8_UINT,
    DXGI_FORMAT_D32_FLOAT_S8X24_UINT,//D32_SFLOAT_S8_UINT,
};

DXGI_FORMAT FormatToNative(format Format)
{
    return FormatTable[(u64)Format];
}


D3D12_RESOURCE_STATES ImageUsageToResourceState(imageUsage::bits Usage)
{
    switch (Usage)
    {
    case imageUsage::UNKNOWN:
        return D3D12_RESOURCE_STATE_COMMON;
    case imageUsage::TRANSFER_SOURCE:
        return D3D12_RESOURCE_STATE_COPY_SOURCE;
    case imageUsage::TRANSFER_DESTINATION:
        return D3D12_RESOURCE_STATE_COPY_DEST;
    case imageUsage::SHADER_READ:
        return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
    case imageUsage::STORAGE:
        return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
    case imageUsage::COLOR_ATTACHMENT:
        return D3D12_RESOURCE_STATE_RENDER_TARGET;
    case imageUsage::DEPTH_STENCIL_ATTACHMENT:
        return D3D12_RESOURCE_STATE_DEPTH_WRITE;
    case imageUsage::INPUT_ATTACHMENT:
        return D3D12_RESOURCE_STATE_COMMON; // ???
    case imageUsage::FRAGNENT_SHADING_RATE_ATTACHMENT:
        return D3D12_RESOURCE_STATE_SHADING_RATE_SOURCE;
    default:
        assert(false);
        return D3D12_RESOURCE_STATE_COMMON;
    }    
}

static D3D12_COMPARISON_FUNC DepthFuncTable[]
{
    D3D12_COMPARISON_FUNC_NEVER,//Never,
    D3D12_COMPARISON_FUNC_LESS,//Less,
    D3D12_COMPARISON_FUNC_EQUAL,//Equal,
    D3D12_COMPARISON_FUNC_LESS_EQUAL,//LessOrEqual,
    D3D12_COMPARISON_FUNC_GREATER,//Greater,
    D3D12_COMPARISON_FUNC_NOT_EQUAL,//NotEqual,
    D3D12_COMPARISON_FUNC_GREATER_EQUAL,//GreaterOrEqual,
    D3D12_COMPARISON_FUNC_ALWAYS,//Always
};

D3D12_COMPARISON_FUNC DepthFuncToNative(compareOperation Operation)
{
    return DepthFuncTable[(sz)Operation];
}

D3D12_DEPTH_WRITE_MASK DepthWriteToNative(u8 DepthWriteEnabled)
{
    return DepthWriteEnabled ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
}

static D3D12_STENCIL_OP StencilOpTable[] = 
{
    D3D12_STENCIL_OP_KEEP,//Keep,
    D3D12_STENCIL_OP_ZERO,//Zero,
    D3D12_STENCIL_OP_REPLACE,//Replace,
    D3D12_STENCIL_OP_INCR,//IncrementAndClamp,
    D3D12_STENCIL_OP_DECR,//DecrementAndClamp,
    D3D12_STENCIL_OP_INVERT,//Invert,
    D3D12_STENCIL_OP_INCR_SAT,//IncrementAndWrap,
    D3D12_STENCIL_OP_DECR_SAT,//DecrementAndWrap    
};

D3D12_STENCIL_OP StencilOpToNative(stencilOperation Operation)
{
    return StencilOpTable[(sz)Operation];
}

D3D12_DEPTH_STENCILOP_DESC StencilStateToNative(stencilOperationState Operation)
{
    D3D12_DEPTH_STENCILOP_DESC Result;
    Result.StencilDepthFailOp = StencilOpToNative(Operation.DepthFail);
    Result.StencilFailOp = StencilOpToNative(Operation.Fail);
    Result.StencilPassOp = StencilOpToNative(Operation.Pass);
    Result.StencilFunc = DepthFuncToNative(Operation.Compare);
    return Result;
}

static D3D12_BLEND BlendFactorTable[] = 
{
    D3D12_BLEND_ZERO,//Zero,
    D3D12_BLEND_ONE,//One,
    D3D12_BLEND_SRC_COLOR,//SrcColor,
    D3D12_BLEND_INV_SRC_COLOR,//OneMinusSrcColor,
    D3D12_BLEND_DEST_COLOR,//DstColor,
    D3D12_BLEND_INV_DEST_COLOR,//OneMinusDstColor,
    D3D12_BLEND_SRC_ALPHA,//SrcAlpha,
    D3D12_BLEND_INV_SRC_ALPHA,//OneMinusSrcAlpha,
    D3D12_BLEND_DEST_ALPHA,//DstAlpha,
    D3D12_BLEND_INV_DEST_ALPHA,//OneMinusDstAlpha,
    (D3D12_BLEND)-1,//ConstantColor,
    (D3D12_BLEND)-1,//OneMinusConstantColor,
    (D3D12_BLEND)-1,//ConstantAlpha,
    (D3D12_BLEND)-1,//OneMinusConstantAlpha,
    D3D12_BLEND_SRC_ALPHA_SAT,//SrcAlphaSaturate,
    D3D12_BLEND_SRC1_COLOR,//Src1Color,
    D3D12_BLEND_INV_SRC1_COLOR,//OneMinusSrc1Color,
    D3D12_BLEND_SRC1_ALPHA,//Src1Alpha,
    D3D12_BLEND_INV_SRC1_ALPHA,//OneMinusSrc1Alpha    
};

D3D12_BLEND BlendFactorToNative(blendFactor Factor)
{
    return BlendFactorTable[(sz)Factor];
}

static D3D12_BLEND_OP BlendOpTable[] = 
{
    D3D12_BLEND_OP_ADD,//Add,
    D3D12_BLEND_OP_SUBTRACT,//Subtract,
    D3D12_BLEND_OP_REV_SUBTRACT,//ReverseSubtract,
    D3D12_BLEND_OP_MIN,//Min,
    D3D12_BLEND_OP_MAX,//Max,
    (D3D12_BLEND_OP)-1,//ZeroEXT,
    (D3D12_BLEND_OP)-1,//SrcEXT,
    (D3D12_BLEND_OP)-1,//DstEXT,
    (D3D12_BLEND_OP)-1,//SrcOverEXT,
    (D3D12_BLEND_OP)-1,//DstOverEXT,
    (D3D12_BLEND_OP)-1,//SrcInEXT,
    (D3D12_BLEND_OP)-1,//DstInEXT,
    (D3D12_BLEND_OP)-1,//SrcOutEXT,
    (D3D12_BLEND_OP)-1,//DstOutEXT,
    (D3D12_BLEND_OP)-1,//SrcAtopEXT,
    (D3D12_BLEND_OP)-1,//DstAtopEXT,
    (D3D12_BLEND_OP)-1,//XorEXT,
    (D3D12_BLEND_OP)-1,//MultiplyEXT,
    (D3D12_BLEND_OP)-1,//ScreenEXT,
    (D3D12_BLEND_OP)-1,//OverlayEXT,
    (D3D12_BLEND_OP)-1,//DarkenEXT,
    (D3D12_BLEND_OP)-1,//LightenEXT,
    (D3D12_BLEND_OP)-1,//ColordodgeEXT,
    (D3D12_BLEND_OP)-1,//ColorburnEXT,
    (D3D12_BLEND_OP)-1,//HardlightEXT,
    (D3D12_BLEND_OP)-1,//SoftlightEXT,
    (D3D12_BLEND_OP)-1,//DifferenceEXT,
    (D3D12_BLEND_OP)-1,//ExclusionEXT,
    (D3D12_BLEND_OP)-1,//InvertEXT,
    (D3D12_BLEND_OP)-1,//InvertRgbEXT,
    (D3D12_BLEND_OP)-1,//LineardodgeEXT,
    (D3D12_BLEND_OP)-1,//LinearburnEXT,
    (D3D12_BLEND_OP)-1,//VividlightEXT,
    (D3D12_BLEND_OP)-1,//LinearlightEXT,
    (D3D12_BLEND_OP)-1,//PinlightEXT,
    (D3D12_BLEND_OP)-1,//HardmixEXT,
    (D3D12_BLEND_OP)-1,//HslHueEXT,
    (D3D12_BLEND_OP)-1,//HslSaturationEXT,
    (D3D12_BLEND_OP)-1,//HslColorEXT,
    (D3D12_BLEND_OP)-1,//HslLuminosityEXT,
    (D3D12_BLEND_OP)-1,//PlusEXT,
    (D3D12_BLEND_OP)-1,//PlusClampedEXT,
    (D3D12_BLEND_OP)-1,//PlusClampedAlphaEXT,
    (D3D12_BLEND_OP)-1,//PlusDarkerEXT,
    (D3D12_BLEND_OP)-1,//MinusEXT,
    (D3D12_BLEND_OP)-1,//MinusClampedEXT,
    (D3D12_BLEND_OP)-1,//ContrastEXT,
    (D3D12_BLEND_OP)-1,//InvertOvgEXT,
    (D3D12_BLEND_OP)-1,//RedEXT,
    (D3D12_BLEND_OP)-1,//GreenEXT,
    (D3D12_BLEND_OP)-1,//BlueEXT    
};

D3D12_BLEND_OP BlendOperationToNative(blendOperation Operation)
{
    return BlendOpTable[(sz)Operation];
}

u8 BlendWriteMaskToNative(colorWriteEnabled::mask Mask)
{
    u8 Result = 0;
    if(Mask & colorWriteEnabled::RedMask)
        Result |= D3D12_COLOR_WRITE_ENABLE_RED;
    if(Mask & colorWriteEnabled::GreenMask)
        Result |= D3D12_COLOR_WRITE_ENABLE_GREEN;
    if(Mask & colorWriteEnabled::BlueMask)
        Result |= D3D12_COLOR_WRITE_ENABLE_BLUE;
    if(Mask & colorWriteEnabled::AlphaMask)
        Result |= D3D12_COLOR_WRITE_ENABLE_ALPHA;

    return Result;
}


D3D12_CULL_MODE CullModeToNative(cullMode::bits Mode)
{
    return (D3D12_CULL_MODE)Mode;
}

b8 FrontFaceToNative(frontFace Face)
{
    return (Face == frontFace::CounterClockwise) ? 1 : 0;
}

static D3D12_FILL_MODE FillModeTable[] = 
{
    D3D12_FILL_MODE_WIREFRAME,
    D3D12_FILL_MODE_SOLID,
    (D3D12_FILL_MODE)-1,
};

D3D12_FILL_MODE FillModeToNative(fillMode Mode)
{
    return FillModeTable[(sz)Mode];
}



}