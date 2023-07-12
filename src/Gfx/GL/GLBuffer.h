#pragma once
#include "../Include/Buffer.h"

#include <GL/glew.h>
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