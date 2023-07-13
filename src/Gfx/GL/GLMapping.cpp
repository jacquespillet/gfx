#include "GLMapping.h"

namespace gfx
{

static GLenum VertexInputTypeTable[] = 
{
    GL_BYTE,
    GL_UNSIGNED_BYTE,
    GL_SHORT,
    GL_UNSIGNED_SHORT,
    GL_INT,
    GL_UNSIGNED_INT,
    GL_HALF_FLOAT,
    GL_FLOAT,
    GL_DOUBLE,
    GL_FIXED,
};

GLenum VertexAttributeTypeToNative(vertexAttributeType Type)
{
    return VertexInputTypeTable[(sz)Type];
}

GLenum BufferUsageFromUsage(bufferUsage::value Value)
{
    switch (Value)
    {
    case bufferUsage::VertexBuffer:
    case bufferUsage::IndexBuffer:
        return GL_STATIC_DRAW;
        break;
    case bufferUsage::UniformBuffer:
    case bufferUsage::StorageBuffer:
        return GL_DYNAMIC_DRAW;
        break;
    default:
        assert(false);
        break;
    }

    return (GLenum)-1;
}

GLenum BufferTargetFromUsage(bufferUsage::value Usage)
{
    switch (Usage)
    {
    case bufferUsage::VertexBuffer:
        return GL_ARRAY_BUFFER;
        break;
    case bufferUsage::IndexBuffer:
        return GL_ELEMENT_ARRAY_BUFFER;
        break;
    case bufferUsage::UniformBuffer:
        return GL_UNIFORM_BUFFER;
        break;
    case bufferUsage::UniformTexelBuffer:
        return GL_TEXTURE_BUFFER;
        break;
    case bufferUsage::StorageBuffer:
        return GL_SHADER_STORAGE_BUFFER;
        break;
    default:
        assert(false);
        break;
    }

    return (GLenum)-1;
}

GLenum ShaderStageToNative(shaderStageFlags::value Stage)
{
    return (GLenum)Stage;
}

static GLenum FormatTableInternal[] = 
{
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    GL_R8,
    GL_R8_SNORM,
    GL_R8,
    GL_R8,
    GL_R8UI,
    GL_R8_SNORM,
    GL_R8,
    GL_RG8,
    GL_RG8_SNORM,
    GL_RG8_SNORM,
    GL_RG8_SNORM,
    GL_RG8UI,
    GL_RG8I,
    GL_RG8,
    GL_RGB8,
    GL_RGB8_SNORM,
    GL_RGB8_SNORM,
    GL_RGB8_SNORM,
    GL_RGB8UI,
    GL_RGB8I,
    GL_SRGB8,
    GL_BGR,
    GL_BGR,
    GL_BGR,
    GL_BGR,
    GL_BGR,
    GL_BGR,
    GL_BGR,
    GL_RGBA8,
    GL_RGBA8_SNORM,
    GL_RGBA8_SNORM,
    GL_RGBA8_SNORM,
    GL_RGBA8UI,
    GL_RGBA8I,
    GL_RGBA8,
    GL_BGRA,
    GL_BGRA,
    GL_BGRA,
    GL_BGRA,
    GL_BGRA,
    GL_BGRA,
    GL_BGRA,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    GL_R16,
    GL_R16_SNORM,
    GL_R16_SNORM,
    GL_R16_SNORM,
    GL_R16UI,
    GL_R16I,
    GL_R16F,
    GL_RG16,
    GL_RG16_SNORM,
    GL_RG16_SNORM,
    GL_RG16_SNORM,
    GL_RG16UI,
    GL_RG16I,
    GL_RG16F,
    GL_RGB16,
    GL_RGB16_SNORM,
    GL_RGB16_SNORM,
    GL_RGB16_SNORM,
    GL_RGB16UI,
    GL_RGB16I,
    GL_RGB16F,
    GL_RGBA16,
    GL_RGBA16_SNORM,
    GL_RGBA16_SNORM,
    GL_RGBA16_SNORM,
    GL_RGBA16UI,
    GL_RGBA16I,
    GL_RGBA16F,
    GL_R32UI,
    GL_R32I,
    GL_R32F,
    GL_RG32UI,
    GL_RG32I,
    GL_RG32F,
    GL_RGB32UI,
    GL_RGB32I,
    GL_RGB32F,
    GL_RGBA32UI,
    GL_RGBA32I,
    GL_RGBA32F,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    GL_DEPTH_COMPONENT16,
    (GLenum)-1,
    GL_DEPTH_COMPONENT32F,
    GL_STENCIL_INDEX8,
    GL_DEPTH_COMPONENT16,
    GL_DEPTH24_STENCIL8,
    GL_DEPTH32F_STENCIL8,      
};


GLenum FormatToNativeInternal(format Format)
{
    GLenum Result =FormatTableInternal[(u64)Format]; 
    return Result;
}


static GLenum FormatTable[] = 
{
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    GL_RED,
    GL_RED,
    GL_RED,
    GL_RED,
    GL_RED_INTEGER,
    GL_RED_INTEGER,
    GL_RED_INTEGER,
    GL_RG,
    GL_RG,
    GL_RG,
    GL_RG,
    GL_RG_INTEGER,
    GL_RG_INTEGER,
    GL_RG_INTEGER,
    GL_RG_INTEGER,
    GL_RGB,
    GL_RGB,
    GL_RGB,
    GL_RGB_INTEGER,
    GL_RGB_INTEGER,
    GL_RGB,
    GL_BGR,
    GL_BGR,
    GL_BGR,
    GL_BGR,
    GL_BGR,
    GL_BGR,
    GL_BGR,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA_INTEGER,
    GL_RGBA_INTEGER,
    GL_RGBA_INTEGER,
    GL_BGRA,
    GL_BGRA,
    GL_BGRA,
    GL_BGRA,
    GL_BGRA,
    GL_BGRA,
    GL_BGRA,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    GL_RED,
    GL_RED,
    GL_RED,
    GL_RED,
    GL_RED_INTEGER,
    GL_RED_INTEGER,
    GL_RED,
    GL_RG,
    GL_RG,
    GL_RG,
    GL_RG,
    GL_RG_INTEGER,
    GL_RG_INTEGER,
    GL_RG,
    GL_RGB,
    GL_RGB,
    GL_RGB,
    GL_RGB,
    GL_RGB_INTEGER,
    GL_RGB_INTEGER,
    GL_RGB,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA,
    GL_RGBA_INTEGER,
    GL_RGBA,
    GL_RGBA,
    GL_RED,
    GL_RED_INTEGER,
    GL_RED,
    GL_RG_INTEGER,
    GL_RG_INTEGER,
    GL_RG,
    GL_RGB_INTEGER,
    GL_RGB_INTEGER,
    GL_RGB,
    GL_RGBA_INTEGER,
    GL_RGBA_INTEGER,
    GL_RGBA,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    (GLenum)-1,
    GL_DEPTH_COMPONENT,
    (GLenum)-1,
    GL_DEPTH_COMPONENT,
    GL_STENCIL_INDEX,
    GL_DEPTH_COMPONENT,
    GL_DEPTH_STENCIL,
    GL_DEPTH_STENCIL,      
};

GLenum FormatToNative(format Format)
{
    GLenum Result = FormatTable[(u64)Format];
    return Result;
}

static GLenum TypeTable[] = 
{
    GL_BYTE,
    GL_UNSIGNED_BYTE,
    GL_SHORT,
    GL_UNSIGNED_SHORT,
    GL_INT,
    GL_UNSIGNED_INT,
    GL_HALF_FLOAT,
    GL_FLOAT,
    GL_DOUBLE,
    GL_FIXED,
    GL_INT_2_10_10_10_REV,
    GL_UNSIGNED_INT_2_10_10_10_REV,
    GL_UNSIGNED_INT_10F_11F_11F_REV,
};

GLenum TypeToNative(type Type)
{
    GLenum Result = TypeTable[(u64)Type];
    return Result;
}

static GLenum FormatToTypeTable[] = 
{
    GL_UNSIGNED_BYTE,//UNDEFINED = 0
    GL_UNSIGNED_SHORT_4_4_4_4,//R4G4_UNORM_PACK_8
    GL_UNSIGNED_SHORT_4_4_4_4,//R4G4B4A4_UNORM_PACK_16
    GL_UNSIGNED_SHORT_4_4_4_4,//B4G4R4A4_UNORM_PACK_16
    GL_UNSIGNED_SHORT_5_5_5_1,//R5G6B5_UNORM_PACK_16
    GL_UNSIGNED_SHORT_5_5_5_1,//B5G6R5_UNORM_PACK_16
    GL_UNSIGNED_SHORT_5_5_5_1,//R5G5B5A1_UNORM_PACK_16
    GL_UNSIGNED_SHORT_5_5_5_1,//B5G5R5A1_UNORM_PACK_16
    GL_UNSIGNED_SHORT_5_5_5_1,//A1R5G5B5_UNORM_PACK_16
    GL_UNSIGNED_BYTE,//R8_UNORM
    GL_BYTE,//R8_SNORM
    GL_BYTE,//R8_USCALED
    GL_BYTE,//R8_SSCALED
    GL_UNSIGNED_BYTE,//R8_UINT
    GL_BYTE,//R8_SINT
    GL_UNSIGNED_BYTE,//R8_SRGB
    GL_UNSIGNED_BYTE,//R8G8_UNORM
    GL_BYTE,//R8G8_SNORM
    GL_UNSIGNED_BYTE,//R8G8_USCALED
    GL_UNSIGNED_BYTE,//R8G8_SSCALED
    GL_UNSIGNED_BYTE,//R8G8_UINT
    GL_BYTE,//R8G8_SINT
    GL_BYTE,//R8G8_SRGB
    GL_UNSIGNED_BYTE,//R8G8B8_UNORM
    GL_BYTE,//R8G8B8_SNORM
    GL_UNSIGNED_BYTE,//R8G8B8_USCALED
    GL_BYTE,//R8G8B8_SSCALED
    GL_UNSIGNED_BYTE,//R8G8B8_UINT
    GL_BYTE,//R8G8B8_SINT
    GL_UNSIGNED_BYTE,//R8G8B8_SRGB
    GL_UNSIGNED_BYTE,//B8G8R8_UNORM
    GL_BYTE,//B8G8R8_SNORM
    GL_UNSIGNED_BYTE,//B8G8R8_USCALED
    GL_BYTE,//B8G8R8_SSCALED
    GL_UNSIGNED_BYTE,//B8G8R8_UINT
    GL_BYTE,//B8G8R8_SINT
    GL_UNSIGNED_BYTE,//B8G8R8_SRGB
    GL_UNSIGNED_BYTE,//R8G8B8A8_UNORM
    GL_BYTE,//R8G8B8A8_SNORM
    GL_UNSIGNED_BYTE,//R8G8B8A8_USCALED
    GL_BYTE,//R8G8B8A8_SSCALED
    GL_UNSIGNED_BYTE,//R8G8B8A8_UINT
    GL_BYTE,//R8G8B8A8_SINT
    GL_UNSIGNED_BYTE,//R8G8B8A8_SRGB
    GL_UNSIGNED_BYTE,//B8G8R8A8_UNORM
    GL_BYTE,//B8G8R8A8_SNORM
    GL_UNSIGNED_BYTE,//B8G8R8A8_USCALED
    GL_BYTE,//B8G8R8A8_SSCALED
    GL_UNSIGNED_BYTE,//B8G8R8A8_UINT
    GL_BYTE,//B8G8R8A8_SINT
    GL_UNSIGNED_BYTE,//B8G8R8A8_SRGB
    GL_UNSIGNED_BYTE,//A8B8G8R8_UNORM_PACK_32
    GL_BYTE,//A8B8G8R8_SNORM_PACK_32
    GL_UNSIGNED_BYTE,//A8B8G8R8_USCALED_PACK_32
    GL_BYTE,//A8B8G8R8_SSCALED_PACK_32
    GL_UNSIGNED_BYTE,//A8B8G8R8_UINT_PACK_32
    GL_BYTE,//A8B8G8R8_SINT_PACK_32
    GL_BYTE,//A8B8G8R8_SRGB_PACK_32
    GL_UNSIGNED_INT_10_10_10_2,//A2R10G10B10_UNORM_PACK_32
    GL_UNSIGNED_INT_10_10_10_2,//A2R10G10B10_SNORM_PACK_32
    GL_UNSIGNED_INT_10_10_10_2,//A2R10G10B10_USCALED_PACK_32
    GL_UNSIGNED_INT_10_10_10_2,//A2R10G10B10_SSCALED_PACK_32
    GL_UNSIGNED_INT_10_10_10_2,//A2R10G10B10_UINT_PACK_32
    GL_UNSIGNED_INT_10_10_10_2,//A2R10G10B10_SINT_PACK_32
    GL_UNSIGNED_INT_10_10_10_2,//A2B10G10R10_UNORM_PACK_32
    GL_UNSIGNED_INT_10_10_10_2,//A2B10G10R10_SNORM_PACK_32
    GL_UNSIGNED_INT_10_10_10_2,//A2B10G10R10_USCALED_PACK_32
    GL_UNSIGNED_INT_10_10_10_2,//A2B10G10R10_SSCALED_PACK_32
    GL_UNSIGNED_INT_10_10_10_2,//A2B10G10R10_UINT_PACK_32
    GL_UNSIGNED_INT_10_10_10_2,//A2B10G10R10_SINT_PACK_32
    GL_UNSIGNED_SHORT,//R16_UNORM
    GL_SHORT,//R16_SNORM
    GL_UNSIGNED_SHORT,//R16_USCALED
    GL_SHORT,//R16_SSCALED
    GL_UNSIGNED_SHORT,//R16_UINT
    GL_SHORT,//R16_SINT
    GL_FLOAT,//R16_SFLOAT
    GL_UNSIGNED_SHORT,//R16G16_UNORM
    GL_SHORT,//R16G16_SNORM
    GL_UNSIGNED_SHORT,//R16G16_USCALED
    GL_SHORT,//R16G16_SSCALED
    GL_UNSIGNED_SHORT,//R16G16_UINT
    GL_SHORT,//R16G16_SINT
    GL_FLOAT,//R16G16_SFLOAT
    GL_UNSIGNED_SHORT,//R16G16B16_UNORM
    GL_SHORT,//R16G16B16_SNORM
    GL_UNSIGNED_SHORT,//R16G16B16_USCALED
    GL_SHORT,//R16G16B16_SSCALED
    GL_UNSIGNED_SHORT,//R16G16B16_UINT
    GL_SHORT,//R16G16B16_SINT
    GL_FLOAT,//R16G16B16_SFLOAT
    GL_UNSIGNED_SHORT,//R16G16B16A16_UNORM
    GL_SHORT,//R16G16B16A16_SNORM
    GL_UNSIGNED_SHORT,//R16G16B16A16_USCALED
    GL_SHORT,//R16G16B16A16_SSCALED
    GL_UNSIGNED_SHORT,//R16G16B16A16_UINT
    GL_SHORT,//R16G16B16A16_SINT
    GL_FLOAT,//R16G16B16A16_SFLOAT
    GL_UNSIGNED_INT,//R32_UINT
    GL_INT,//R32_SINT
    GL_FLOAT,//R32_SFLOAT
    GL_UNSIGNED_INT,//R32G32_UINT
    GL_INT,//R32G32_SINT
    GL_FLOAT,//R32G32_SFLOAT
    GL_UNSIGNED_INT,//R32G32B32_UINT
    GL_INT,//R32G32B32_SINT
    GL_FLOAT,//R32G32B32_SFLOAT
    GL_UNSIGNED_INT,//R32G32B32A32_UINT
    GL_INT,//R32G32B32A32_SINT
    GL_FLOAT,//R32G32B32A32_SFLOAT
    GL_UNSIGNED_INT,//R64_UINT
    GL_INT,//R64_SINT
    GL_FLOAT,//R64_SFLOAT
    GL_UNSIGNED_INT,//R64G64_UINT
    GL_INT,//R64G64_SINT
    GL_FLOAT,//R64G64_SFLOAT
    GL_UNSIGNED_INT,//R64G64B64_UINT
    GL_INT,//R64G64B64_SINT
    GL_FLOAT,//R64G64B64_SFLOAT
    GL_UNSIGNED_INT,//R64G64B64A64_UINT
    GL_INT,//R64G64B64A64_SINT
    GL_FLOAT,//R64G64B64A64_SFLOAT
    GL_FLOAT,//B10G11R11_UFLOAT_PACK_32
    GL_FLOAT,//E5B9G9R9_UFLOAT_PACK_32
    GL_UNSIGNED_SHORT,//D16_UNORM
    GL_FLOAT,//X8D24_UNORM_PACK_32
    GL_FLOAT,//D32_SFLOAT
    GL_UNSIGNED_BYTE,//S8_UINT
    GL_UNSIGNED_SHORT_8_8_APPLE,//D16_UNORM_S8_UINT
    GL_UNSIGNED_INT_24_8,//D24_UNORM_S8_UINT
    GL_FLOAT,//D32_SFLOAT_S8_UINT
};

GLenum FormatToType(format Format)
{
    GLenum Result = FormatToTypeTable[(sz)Format];
    return Result;
}


static GLenum SamplerWrapTable[] = 
{
    GL_CLAMP_TO_EDGE,//ClampToEdge,
    GL_CLAMP_TO_BORDER,//ClampToBorder,
    GL_MIRRORED_REPEAT_ARB,//MirroredRepeat,
    GL_REPEAT,//Repeat,
    GL_MIRROR_CLAMP_TO_EDGE,//MirrorClampToEdge
};
GLenum SamplerWrapToNative(samplerWrapMode Mode)
{
    return SamplerWrapTable[(sz)Mode];
}


static GLenum SamplerFilterTable[] = 
{
    GL_NEAREST,//Nearest,
    GL_LINEAR,//Linear,
    GL_NEAREST_MIPMAP_NEAREST,//NearestMipmapNearest,
    GL_LINEAR_MIPMAP_NEAREST,//LinearMipmapNearest,
    GL_NEAREST_MIPMAP_NEAREST,//NearestMipmapLinear,
    GL_LINEAR_MIPMAP_LINEAR,//LinearMipmapLinear
};
GLenum SamplerFilterToNative(samplerFilter Filter)
{
    return SamplerFilterTable[(sz)Filter];
}

static GLenum StencilOpTable[] = 
{
    GL_KEEP,//Keep,
    GL_ZERO,//Zero,
    GL_REPLACE,//Replace,
    GL_INCR,//IncrementAndClamp,
    GL_DECR,//DecrementAndClamp,
    GL_INVERT,//Invert,
    GL_INCR_WRAP,//IncrementAndWrap,
    GL_DECR_WRAP,//DecrementAndWrap    
};

GLenum StencilOpToNative(stencilOperation Op)
{
    return StencilOpTable[(sz)Op];
}

glStencilOperation StencilStateToNative(stencilOperationState Op)
{
    glStencilOperation Result = {};
    Result.DepthFail = StencilOpToNative(Op.DepthFail);
    Result.Fail = StencilOpToNative(Op.Fail);
    Result.Pass = StencilOpToNative(Op.Pass);
    Result.Operation = CompareOpToNative(Op.Compare);
    return Result;
}

static GLenum CompareOpTable[] = 
{
    GL_NEVER,//Never,
    GL_LESS,//Less,
    GL_EQUAL,//Equal,
    GL_LEQUAL,//LessOrEqual,
    GL_GREATER,//Greater,
    GL_NOTEQUAL,//NotEqual,
    GL_GEQUAL,//GreaterOrEqual,
    GL_ALWAYS,//Always    
};

GLenum CompareOpToNative(compareOperation Op)
{
    return CompareOpTable[(sz)Op];
}

static GLenum BlendFactorTable[] =
{
    GL_ZERO,//Zero,
    GL_ONE,//One,
    GL_SRC_COLOR,//SrcColor,
    GL_ONE_MINUS_SRC_COLOR,//OneMinusSrcColor,
    GL_DST_COLOR,//DstColor,
    GL_ONE_MINUS_DST_COLOR,//OneMinusDstColor,
    GL_SRC_ALPHA,//SrcAlpha,
    GL_ONE_MINUS_SRC_ALPHA,//OneMinusSrcAlpha,
    GL_DST_ALPHA,//DstAlpha,
    GL_ONE_MINUS_DST_ALPHA,//OneMinusDstAlpha,
    GL_CONSTANT_COLOR,//ConstantColor,
    GL_ONE_MINUS_CONSTANT_COLOR,//OneMinusConstantColor,
    GL_CONSTANT_ALPHA,//ConstantAlpha,
    GL_ONE_MINUS_CONSTANT_ALPHA,//OneMinusConstantAlpha,
    GL_SRC_ALPHA_SATURATE_EXT,//SrcAlphaSaturate,
    GL_SRC1_COLOR,//Src1Color,
    GL_ONE_MINUS_SRC1_COLOR,//OneMinusSrc1Color,
    GL_SRC1_ALPHA,//Src1Alpha,
    GL_ONE_MINUS_SRC1_ALPHA,//OneMinusSrc1Alpha    
};

GLenum BlendFactorToNative(blendFactor Factor)
{
    return BlendFactorTable[(sz)Factor];
}

static GLenum BlendOpTablep[] = 
{
    GL_ADD,//Add,
    GL_SUBTRACT,//Subtract,
    GL_FUNC_REVERSE_SUBTRACT,//ReverseSubtract,
    GL_MIN,//Min,
    GL_MAX,//Max,
    GL_ZERO,//ZeroEXT,
    (GLenum)-1,//SrcEXT,
    (GLenum)-1,//DstEXT,
    (GLenum)-1,//SrcOverEXT,
    (GLenum)-1,//DstOverEXT,
    (GLenum)-1,//SrcInEXT,
    (GLenum)-1,//DstInEXT,
    (GLenum)-1,//SrcOutEXT,
    (GLenum)-1,//DstOutEXT,
    (GLenum)-1,//SrcAtopEXT,
    (GLenum)-1,//DstAtopEXT,
    (GLenum)-1,//XorEXT,
    (GLenum)-1,//MultiplyEXT,
    (GLenum)-1,//ScreenEXT,
    (GLenum)-1,//OverlayEXT,
    (GLenum)-1,//DarkenEXT,
    (GLenum)-1,//LightenEXT,
    (GLenum)-1,//ColordodgeEXT,
    (GLenum)-1,//ColorburnEXT,
    (GLenum)-1,//HardlightEXT,
    (GLenum)-1,//SoftlightEXT,
    (GLenum)-1,//DifferenceEXT,
    (GLenum)-1,//ExclusionEXT,
    (GLenum)-1,//InvertEXT,
    (GLenum)-1,//InvertRgbEXT,
    (GLenum)-1,//LineardodgeEXT,
    (GLenum)-1,//LinearburnEXT,
    (GLenum)-1,//VividlightEXT,
    (GLenum)-1,//LinearlightEXT,
    (GLenum)-1,//PinlightEXT,
    (GLenum)-1,//HardmixEXT,
    (GLenum)-1,//HslHueEXT,
    (GLenum)-1,//HslSaturationEXT,
    (GLenum)-1,//HslColorEXT,
    (GLenum)-1,//HslLuminosityEXT,
    (GLenum)-1,//PlusEXT,
    (GLenum)-1,//PlusClampedEXT,
    (GLenum)-1,//PlusClampedAlphaEXT,
    (GLenum)-1,//PlusDarkerEXT,
    (GLenum)-1,//MinusEXT,
    (GLenum)-1,//MinusClampedEXT,
    (GLenum)-1,//ContrastEXT,
    (GLenum)-1,//InvertOvgEXT,
    (GLenum)-1,//RedEXT,
    (GLenum)-1,//GreenEXT,
    (GLenum)-1,//BlueEXT
};

GLenum BlendOpToNative(blendOperation Op)
{
    return BlendOpTablep[(sz)Op];
}

GLenum CullModeToNative(cullMode::bits Mode)
{
    return (GLenum)Mode;
}

static GLenum FillModeTable[] = 
{
    GL_LINE,//WireFrame, 
    GL_FILL,//Solid, 
    GL_POINT,//Point, 
};

GLenum FillModeToNative(fillMode Mode)
{
    return FillModeTable[(sz)Mode];
}

GLenum FrontFaceToNative(frontFace Mode)
{
    return (Mode == frontFace::Clockwise) ? GL_CW : GL_CCW;
}


}