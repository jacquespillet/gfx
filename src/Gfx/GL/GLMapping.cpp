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
}

GLenum ShaderStageToNative(shaderStageFlags::value Stage)
{
    return (GLenum)Stage;
}

static GLenum FormatTableInternal[] = 
{
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
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
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
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
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    GL_DEPTH_COMPONENT16,
    -1,
    GL_DEPTH_COMPONENT32F,
    GL_STENCIL_INDEX8,
    GL_DEPTH_COMPONENT16,
    GL_DEPTH24_STENCIL8,
    GL_DEPTH32F_STENCIL8,      
};


GLenum FormatToNativeInternal(format Format)
{
    return FormatTableInternal[(u64)Format];
}


static GLenum FormatTable[] = 
{
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
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
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
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
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    GL_DEPTH_COMPONENT,
    -1,
    GL_DEPTH_COMPONENT,
    GL_STENCIL_INDEX,
    GL_DEPTH_COMPONENT,
    GL_DEPTH_STENCIL,
    GL_DEPTH_STENCIL,      
};

GLenum FormatToNative(format Format)
{
    return FormatTable[(u64)Format];
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
    return TypeTable[(u64)Type];
}


}