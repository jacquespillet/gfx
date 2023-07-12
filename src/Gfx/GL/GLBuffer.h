#pragma once
#include <GL/glew.h>
namespace gfx
{

struct glBuffer
{
    GLuint Handle;
    GLuint VAO = (GLuint)-1; //Used if the buffer is a vertex buffer
    GLenum Target;
    GLenum Usage;
};

}