#pragma once
#include "Types.h"

namespace gfx
{
    
enum class attributeSemantic
{
    POSITION,
    NORMAL,
    TANGENT,
    BITANGENT,
    COLOR,
    UV0,
    UV1,
    UV2,
    OTHER
};   

enum class vertexAttributeType : u32
{
    byte,
    UnsignedByte,
    Short,
    UnsignedShort,
    Int,
    UnsignedInt,
    HalfFloat,
    Float,
    Double,
    Fixed
};

struct vertexInputAttribute
{
    u8 ElementSize;
    u8 ElementCount;
    vertexAttributeType Type;
    b8 Normalized;
    attributeSemantic AttributeType;
    u32 StreamIndex;
    u32 InputIndex;
};

}