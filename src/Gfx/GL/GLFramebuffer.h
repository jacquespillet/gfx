#pragma once

#include <glad/gl.h>

namespace gfx
{
struct glFramebufferData
{
    GLuint Handle;

    std::vector<GLuint> ColorTextures;
    GLuint DepthTexture;
};  
}