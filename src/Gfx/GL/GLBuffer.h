#pragma once
#include <GL/glew.h>
namespace gfx
{

struct glBuffer
{
    GLuint Handle;
    GLuint VAO; //Used if the buffer is a vertex buffer
    GLenum Target;
    GLenum Usage;
};

}