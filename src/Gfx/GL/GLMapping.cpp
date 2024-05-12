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
    (GLenum)-1, //UNDEFINED = 0,
    (GLenum)-1, //R4G4_UNORM_PACK_8,
    (GLenum)-1, //R4G4B4A4_UNORM_PACK_16,
    (GLenum)-1, //B4G4R4A4_UNORM_PACK_16,
    (GLenum)-1, //R5G6B5_UNORM_PACK_16,
    (GLenum)-1, //B5G6R5_UNORM_PACK_16,
    (GLenum)-1, //R5G5B5A1_UNORM_PACK_16,
    (GLenum)-1, //B5G5R5A1_UNORM_PACK_16,
    (GLenum)-1, //A1R5G5B5_UNORM_PACK_16,
    GL_R8, //R8_UNORM,
    GL_R8_SNORM, //R8_SNORM,
    GL_R8, //R8_USCALED,
    GL_R8, //R8_SSCALED,
    GL_R8UI, //R8_UINT,
    GL_R8_SNORM, //R8_SINT,
    GL_R8, //R8_SRGB,
    GL_RG8, //R8G8_UNORM,
    GL_RG8_SNORM, //R8G8_SNORM,
    GL_RG8_SNORM, //R8G8_USCALED,
    GL_RG8_SNORM, //R8G8_SSCALED,
    GL_RG8UI, //R8G8_UINT,
    GL_RG8I, //R8G8_SINT,
    GL_RG8, //R8G8_SRGB,
    GL_RGB8, //R8G8B8_UNORM,
    GL_RGB8_SNORM, //R8G8B8_SNORM,
    GL_RGB8_SNORM, //R8G8B8_USCALED,
    GL_RGB8_SNORM, //R8G8B8_SSCALED,
    GL_RGB8UI, //R8G8B8_UINT,
    GL_RGB8I, //R8G8B8_SINT,
    GL_SRGB8, //R8G8B8_SRGB,
    GL_BGR, //B8G8R8_UNORM,
    GL_BGR, //B8G8R8_SNORM,
    GL_BGR, //B8G8R8_USCALED,
    GL_BGR, //B8G8R8_SSCALED,
    GL_BGR, //B8G8R8_UINT,
    GL_BGR, //B8G8R8_SINT,
    GL_BGR, //B8G8R8_SRGB,
    GL_RGBA8, //R8G8B8A8_UNORM,
    GL_RGBA8_SNORM, //R8G8B8A8_SNORM,
    GL_RGBA8_SNORM, //R8G8B8A8_USCALED,
    GL_RGBA8_SNORM, //R8G8B8A8_SSCALED,
    GL_RGBA8UI, //R8G8B8A8_UINT,
    GL_RGBA8I, //R8G8B8A8_SINT,
    GL_RGBA8, //R8G8B8A8_SRGB,
    GL_BGRA, //B8G8R8A8_UNORM,
    GL_BGRA, //B8G8R8A8_SNORM,
    GL_BGRA, //B8G8R8A8_USCALED,
    GL_BGRA, //B8G8R8A8_SSCALED,
    GL_BGRA, //B8G8R8A8_UINT,
    GL_BGRA, //B8G8R8A8_SINT,
    GL_BGRA, //B8G8R8A8_SRGB,
    (GLenum)-1, //A8B8G8R8_UNORM_PACK_32,
    (GLenum)-1, //A8B8G8R8_SNORM_PACK_32,
    (GLenum)-1, //A8B8G8R8_USCALED_PACK_32,
    (GLenum)-1, //A8B8G8R8_SSCALED_PACK_32,
    (GLenum)-1, //A8B8G8R8_UINT_PACK_32,
    (GLenum)-1, //A8B8G8R8_SINT_PACK_32,
    (GLenum)-1, //A8B8G8R8_SRGB_PACK_32,
    (GLenum)-1, //A2R10G10B10_UNORM_PACK_32,
    (GLenum)-1, //A2R10G10B10_SNORM_PACK_32,
    (GLenum)-1, //A2R10G10B10_USCALED_PACK_32,
    (GLenum)-1, //A2R10G10B10_SSCALED_PACK_32,
    (GLenum)-1, //A2R10G10B10_UINT_PACK_32,
    (GLenum)-1, //A2R10G10B10_SINT_PACK_32,
    (GLenum)-1, //A2B10G10R10_UNORM_PACK_32,
    (GLenum)-1, //A2B10G10R10_SNORM_PACK_32,
    (GLenum)-1, //A2B10G10R10_USCALED_PACK_32,
    (GLenum)-1, //A2B10G10R10_SSCALED_PACK_32,
    (GLenum)-1, //A2B10G10R10_UINT_PACK_32,
    (GLenum)-1, //A2B10G10R10_SINT_PACK_32,
    GL_R16, //R16_UNORM,
    GL_R16_SNORM, //R16_SNORM,
    GL_R16_SNORM, //R16_USCALED,
    GL_R16_SNORM, //R16_SSCALED,
    GL_R16UI, //R16_UINT,
    GL_R16I, //R16_SINT,
    GL_R16F, //R16_SFLOAT,
    GL_RG16, //R16G16_UNORM,
    GL_RG16_SNORM, //R16G16_SNORM,
    GL_RG16_SNORM, //R16G16_USCALED,
    GL_RG16_SNORM, //R16G16_SSCALED,
    GL_RG16UI, //R16G16_UINT,
    GL_RG16I, //R16G16_SINT,
    GL_RG16F, //R16G16_SFLOAT,
    GL_RGB16, //R16G16B16_UNORM,
    GL_RGB16_SNORM, //R16G16B16_SNORM,
    GL_RGB16_SNORM, //R16G16B16_USCALED,
    GL_RGB16_SNORM, //R16G16B16_SSCALED,
    GL_RGB16UI, //R16G16B16_UINT,
    GL_RGB16I, //R16G16B16_SINT,
    GL_RGB16F, //R16G16B16_SFLOAT,
    GL_RGBA16, //R16G16B16A16_UNORM,
    GL_RGBA16_SNORM, //R16G16B16A16_SNORM,
    GL_RGBA16_SNORM, //R16G16B16A16_USCALED,
    GL_RGBA16_SNORM, //R16G16B16A16_SSCALED,
    GL_RGBA16UI, //R16G16B16A16_UINT,
    GL_RGBA16I, //R16G16B16A16_SINT,
    GL_RGBA16F, //R16G16B16A16_SFLOAT,
    GL_R32UI, //R32_UINT,
    GL_R32I, //R32_SINT,
    GL_R32F, //R32_SFLOAT,
    GL_RG32UI, //R32G32_UINT,
    GL_RG32I, //R32G32_SINT,
    GL_RG32F, //R32G32_SFLOAT,
    GL_RGB32UI, //R32G32B32_UINT,
    GL_RGB32I, //R32G32B32_SINT,
    GL_RGB32F, //R32G32B32_SFLOAT,
    GL_RGBA32UI, //R32G32B32A32_UINT,
    GL_RGBA32I, //R32G32B32A32_SINT,
    GL_RGBA32F, //R32G32B32A32_SFLOAT,
    (GLenum)-1, //R64_UINT,
    (GLenum)-1, //R64_SINT,
    (GLenum)-1, //R64_SFLOAT,
    (GLenum)-1, //R64G64_UINT,
    (GLenum)-1, //R64G64_SINT,
    (GLenum)-1, //R64G64_SFLOAT,
    (GLenum)-1, //R64G64B64_UINT,
    (GLenum)-1, //R64G64B64_SINT,
    (GLenum)-1, //R64G64B64_SFLOAT,
    (GLenum)-1, //R64G64B64A64_UINT,
    (GLenum)-1, //R64G64B64A64_SINT,
    (GLenum)-1, //R64G64B64A64_SFLOAT,
    (GLenum)-1, //B10G11R11_UFLOAT_PACK_32,
    (GLenum)-1, //E5B9G9R9_UFLOAT_PACK_32,
    GL_DEPTH_COMPONENT16, //D16_UNORM,
    (GLenum)-1, //X8D24_UNORM_PACK_32,
    GL_DEPTH_COMPONENT32F, //D32_SFLOAT,
    GL_STENCIL_INDEX8, //S8_UINT,
    GL_DEPTH_COMPONENT16, //D16_UNORM_S8_UINT,
    GL_DEPTH24_STENCIL8, //D24_UNORM_S8_UINT,
    GL_DEPTH32F_STENCIL8,       //D32_SFLOAT_S8_UINT,
};


GLenum FormatToNativeInternal(format Format)
{
    GLenum Result =FormatTableInternal[(u64)Format]; 
    return Result;
}


static GLenum FormatTable[] = 
{
    (GLenum)-1,//UNDEFINED = 0,
    (GLenum)-1,//R4G4_UNORM_PACK_8,
    (GLenum)-1,//R4G4B4A4_UNORM_PACK_16,
    (GLenum)-1,//B4G4R4A4_UNORM_PACK_16,
    (GLenum)-1,//R5G6B5_UNORM_PACK_16,
    (GLenum)-1,//B5G6R5_UNORM_PACK_16,
    (GLenum)-1,//R5G5B5A1_UNORM_PACK_16,
    (GLenum)-1,//B5G5R5A1_UNORM_PACK_16,
    (GLenum)-1,//A1R5G5B5_UNORM_PACK_16,
    GL_RED,//R8_UNORM,
    GL_RED,//R8_SNORM,
    GL_RED,//R8_USCALED,
    GL_RED,//R8_SSCALED,
    GL_RED_INTEGER,//R8_UINT,
    GL_RED_INTEGER,//R8_SINT,
    GL_RED_INTEGER,//R8_SRGB,
    GL_RG,//R8G8_UNORM,
    GL_RG,//R8G8_SNORM,
    GL_RG,//R8G8_USCALED,
    GL_RG,//R8G8_SSCALED,
    GL_RG_INTEGER,//R8G8_UINT,
    GL_RG_INTEGER,//R8G8_SINT,
    GL_RG_INTEGER,//R8G8_SRGB,
    GL_RG_INTEGER,//R8G8B8_UNORM,
    GL_RGB,//R8G8B8_SNORM,
    GL_RGB,//R8G8B8_USCALED,
    GL_RGB,//R8G8B8_SSCALED,
    GL_RGB_INTEGER,//R8G8B8_UINT,
    GL_RGB_INTEGER,//R8G8B8_SINT,
    GL_RGB,//R8G8B8_SRGB,
    GL_BGR,//B8G8R8_UNORM,
    GL_BGR,//B8G8R8_SNORM,
    GL_BGR,//B8G8R8_USCALED,
    GL_BGR,//B8G8R8_SSCALED,
    GL_BGR,//B8G8R8_UINT,
    GL_BGR,//B8G8R8_SINT,
    GL_BGR,//B8G8R8_SRGB,
    GL_RGBA,//R8G8B8A8_UNORM,
    GL_RGBA,//R8G8B8A8_SNORM,
    GL_RGBA,//R8G8B8A8_USCALED,
    GL_RGBA,//R8G8B8A8_SSCALED,
    GL_RGBA_INTEGER,//R8G8B8A8_UINT,
    GL_RGBA_INTEGER,//R8G8B8A8_SINT,
    GL_RGBA_INTEGER,//R8G8B8A8_SRGB,
    GL_BGRA,//B8G8R8A8_UNORM,
    GL_BGRA,//B8G8R8A8_SNORM,
    GL_BGRA,//B8G8R8A8_USCALED,
    GL_BGRA,//B8G8R8A8_SSCALED,
    GL_BGRA,//B8G8R8A8_UINT,
    GL_BGRA,//B8G8R8A8_SINT,
    GL_BGRA,//B8G8R8A8_SRGB,
    (GLenum)-1,//A8B8G8R8_UNORM_PACK_32,
    (GLenum)-1,//A8B8G8R8_SNORM_PACK_32,
    (GLenum)-1,//A8B8G8R8_USCALED_PACK_32,
    (GLenum)-1,//A8B8G8R8_SSCALED_PACK_32,
    (GLenum)-1,//A8B8G8R8_UINT_PACK_32,
    (GLenum)-1,//A8B8G8R8_SINT_PACK_32,
    (GLenum)-1,//A8B8G8R8_SRGB_PACK_32,
    (GLenum)-1,//A2R10G10B10_UNORM_PACK_32,
    (GLenum)-1,//A2R10G10B10_SNORM_PACK_32,
    (GLenum)-1,//A2R10G10B10_USCALED_PACK_32,
    (GLenum)-1,//A2R10G10B10_SSCALED_PACK_32,
    (GLenum)-1,//A2R10G10B10_UINT_PACK_32,
    (GLenum)-1,//A2R10G10B10_SINT_PACK_32,
    (GLenum)-1,//A2B10G10R10_UNORM_PACK_32,
    (GLenum)-1,//A2B10G10R10_SNORM_PACK_32,
    (GLenum)-1,//A2B10G10R10_USCALED_PACK_32,
    (GLenum)-1,//A2B10G10R10_SSCALED_PACK_32,
    (GLenum)-1,//A2B10G10R10_UINT_PACK_32,
    (GLenum)-1,//A2B10G10R10_SINT_PACK_32,
    GL_RED,//R16_UNORM,
    GL_RED,//R16_SNORM,
    GL_RED,//R16_USCALED,
    GL_RED,//R16_SSCALED,
    GL_RED_INTEGER,//R16_UINT,
    GL_RED_INTEGER,//R16_SINT,
    GL_RED,//R16_SFLOAT,
    GL_RG,//R16G16_UNORM,
    GL_RG,//R16G16_SNORM,
    GL_RG,//R16G16_USCALED,
    GL_RG,//R16G16_SSCALED,
    GL_RG_INTEGER,//R16G16_UINT,
    GL_RG_INTEGER,//R16G16_SINT,
    GL_RG,//R16G16_SFLOAT,
    GL_RGB,//R16G16B16_UNORM,
    GL_RGB,//R16G16B16_SNORM,
    GL_RGB,//R16G16B16_USCALED,
    GL_RGB,//R16G16B16_SSCALED,
    GL_RGB_INTEGER,//R16G16B16_UINT,
    GL_RGB_INTEGER,//R16G16B16_SINT,
    GL_RGB,//R16G16B16_SFLOAT,
    GL_RGBA,//R16G16B16A16_UNORM,
    GL_RGBA,//R16G16B16A16_SNORM,
    GL_RGBA,//R16G16B16A16_USCALED,
    GL_RGBA,//R16G16B16A16_SSCALED,
    GL_RGBA_INTEGER,//R16G16B16A16_UINT,
    GL_RGBA,//R16G16B16A16_SINT,
    GL_RGBA,//R16G16B16A16_SFLOAT,
    GL_RED,//R32_UINT,
    GL_RED_INTEGER,//R32_SINT,
    GL_RED,//R32_SFLOAT,
    GL_RG_INTEGER,//R32G32_UINT,
    GL_RG_INTEGER,//R32G32_SINT,
    GL_RG,//R32G32_SFLOAT,
    GL_RGB_INTEGER,//R32G32B32_UINT,
    GL_RGB_INTEGER,//R32G32B32_SINT,
    GL_RGB,//R32G32B32_SFLOAT,
    GL_RGBA_INTEGER,//R32G32B32A32_UINT,
    GL_RGBA_INTEGER,//R32G32B32A32_SINT,
    GL_RGBA,//R32G32B32A32_SFLOAT,
    (GLenum)-1,//R64_UINT,
    (GLenum)-1,//R64_SINT,
    (GLenum)-1,//R64_SFLOAT,
    (GLenum)-1,//R64G64_UINT,
    (GLenum)-1,//R64G64_SINT,
    (GLenum)-1,//R64G64_SFLOAT,
    (GLenum)-1,//R64G64B64_UINT,
    (GLenum)-1,//R64G64B64_SINT,
    (GLenum)-1,//R64G64B64_SFLOAT,
    (GLenum)-1,//R64G64B64A64_UINT,
    (GLenum)-1,//R64G64B64A64_SINT,
    (GLenum)-1,//R64G64B64A64_SFLOAT,
    (GLenum)-1,//B10G11R11_UFLOAT_PACK_32,
    (GLenum)-1,//E5B9G9R9_UFLOAT_PACK_32,
    GL_DEPTH_COMPONENT,//D16_UNORM,
    (GLenum)-1,//X8D24_UNORM_PACK_32,
    GL_DEPTH_COMPONENT,//D32_SFLOAT,
    GL_STENCIL_INDEX,//S8_UINT,
    GL_DEPTH_COMPONENT,//D16_UNORM_S8_UINT,
    GL_DEPTH_STENCIL,//D24_UNORM_S8_UINT,
    GL_DEPTH_STENCIL,      //D32_SFLOAT_S8_UINT,
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
    GL_SRC_ALPHA_SATURATE,//SrcAlphaSaturate,
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
    GL_FUNC_ADD,//Add,
    GL_FUNC_SUBTRACT,//Subtract,
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

GLenum IndexTypeToNative(indexType Type)
{
    return Type == indexType::Uint16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
}

sz IndexTypeSize(indexType Type)
{
    return Type == indexType::Uint16 ? sizeof(uint16_t) : sizeof(uint32_t);
}

}