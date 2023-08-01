#pragma once
#include "GLShader.h"

namespace gfx
{
struct pipelineCreation;
struct glStencilOperation
{
    GLenum Fail = GL_STENCIL_FAIL;
    GLenum Pass = GL_STENCIL_FAIL;
    GLenum DepthFail = GL_STENCIL_FAIL;
    GLenum Operation;
};

struct glPipeline
{
    std::shared_ptr<glShaderProgram> ShaderProgram;


    void Create(const pipelineCreation &PipelineCreation);
    void DestroyGLResources();

    struct depthStencil
    {
        glStencilOperation FrontStencilOp;
        glStencilOperation BackStencilOp;
        b8 DepthEnable;
        b8 DepthWrite;
        b8 StencilEnable;
        GLenum DepthComparison;
    } DepthStencil;

    struct blend
    {
        GLenum BlendSourceColor;
        GLenum BlendDestColor;
        GLenum BlendSourceAlpha;
        GLenum BlendDestAlpha;
        GLenum ColorOp;
        GLenum AlphaOp;

        b8 Enabled;
        b8 Separate;
    } Blend;

    struct rasterizer
    {
        GLenum CullMode;
        GLenum FrontFace;
        GLenum FillMode;
    } Rasterizer;


};

}