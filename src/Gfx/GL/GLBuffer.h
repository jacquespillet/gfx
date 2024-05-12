#pragma once
#include "../Include/Buffer.h"

#include <glad/gl.h>
namespace gfx
{

struct glBuffer
{
    GLuint Handle;
    GLenum Target;
    GLenum Usage;
};

struct glVertexBuffer
{
    GLuint VAO = (GLuint)-1;
    bufferHandle VertexBuffers[commonConstants::MaxVertexStreams];
};

}