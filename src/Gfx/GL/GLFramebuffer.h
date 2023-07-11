#pragma once

#include <GL/glew.h>

namespace gfx
{
struct glFramebufferData
{
    GLuint Handle;

    std::vector<GLuint> ColorTextures;
    GLuint DepthTexture;
};  
}