#pragma once
#include "../Include/Types.h"
#include "../Include/VertexInput.h"
#include "../Include/Image.h"
#include "../Include/Pipeline.h"

#include "GLPipeline.h"

#include <glad/gl.h>
#include <assert.h>

namespace gfx
{
GLenum BufferTargetFromUsage(bufferUsage::value Usage);

GLenum BufferUsageFromUsage(bufferUsage::value Value);

GLenum VertexAttributeTypeToNative(vertexAttributeType Type);

GLenum ShaderStageToNative(shaderStageFlags::value Stage);

GLenum FormatToNativeInternal(format Format);
GLenum FormatToNative(format Format);
GLenum FormatToType(format Format);
GLenum TypeToNative(type Type);

GLenum SamplerWrapToNative(samplerWrapMode Mode);
GLenum SamplerFilterToNative(samplerFilter Filter);

glStencilOperation StencilStateToNative(stencilOperationState Op);
GLenum CompareOpToNative(compareOperation Op);
GLenum BlendFactorToNative(blendFactor Factor);
GLenum BlendOpToNative(blendOperation Op);
GLenum CullModeToNative(cullMode::bits Mode);
GLenum FillModeToNative(fillMode Mode);
GLenum FrontFaceToNative(frontFace Mode);


GLenum IndexTypeToNative(indexType Type);
sz IndexTypeSize(indexType Type);

}