#pragma once
#include "../Include/Types.h"
#include "../Include/VertexInput.h"

#include <GL/glew.h>
#include <assert.h>

namespace gfx
{
GLenum BufferTargetFromUsage(bufferUsage::value Usage);

GLenum BufferUsageFromUsage(bufferUsage::value Value);

GLenum VertexAttributeTypeToNative(vertexAttributeType Type);

GLenum ShaderStageToNative(shaderStageFlags::value Stage);

GLenum FormatToNativeInternal(format Format);
GLenum FormatToNative(format Format);
GLenum TypeToNative(type Type);
}