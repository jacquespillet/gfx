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

}