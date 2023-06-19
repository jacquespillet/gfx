#pragma once
#include "stdint.h"
namespace gfx
{
typedef uint32_t bufferHandle;
typedef uint32_t pipelineHandle;
typedef uint32_t renderPassHandle;


enum class clearBufferType
{
    Color, Depth
};
}