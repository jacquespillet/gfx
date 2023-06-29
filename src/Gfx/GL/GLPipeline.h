#pragma once
#include "GLShader.h"

namespace gfx
{
struct glPipeline
{
    std::shared_ptr<glShaderProgram> ShaderProgram;
};

}