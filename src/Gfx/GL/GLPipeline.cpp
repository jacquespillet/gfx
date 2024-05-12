#include "GLPipeline.h"
#include <glad/gl.h>
#include "GLMapping.h"
#include "../Include/Pipeline.h"
#include "../Include/Memory.h"

namespace gfx
{
void glPipeline::Create(const pipelineCreation &PipelineCreation)
{
    this->ShaderProgram = std::make_shared<glShaderProgram>();
    for (size_t i = 0; i < PipelineCreation.Shaders.StagesCount; i++)
    {
        this->ShaderProgram->SetCodeForStage(PipelineCreation.Shaders.Stages[i].Code, PipelineCreation.Shaders.Stages[i].Stage);
    }
    this->ShaderProgram->Compile();
    

    if(!PipelineCreation.IsCompute)
    {
        //Depth stencil state
        this->DepthStencil.FrontStencilOp = StencilStateToNative(PipelineCreation.DepthStencil.Front);
        this->DepthStencil.BackStencilOp = StencilStateToNative(PipelineCreation.DepthStencil.Back);
        this->DepthStencil.DepthEnable = PipelineCreation.DepthStencil.DepthEnable;
        this->DepthStencil.DepthWrite = PipelineCreation.DepthStencil.DepthWriteEnable;
        this->DepthStencil.StencilEnable = PipelineCreation.DepthStencil.StencilEnable;
        this->DepthStencil.DepthComparison = CompareOpToNative(PipelineCreation.DepthStencil.DepthComparison);

        //Blend state
        if(PipelineCreation.BlendState.ActiveStates>0)
        {
            this->Blend.Enabled=PipelineCreation.BlendState.BlendStates[0].BlendEnabled;
            this->Blend.BlendSourceColor = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[0].SourceColor);
            this->Blend.BlendDestColor = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[0].DestinationColor);
            this->Blend.BlendSourceAlpha = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[0].SourceAlpha);
            this->Blend.BlendDestAlpha = BlendFactorToNative(PipelineCreation.BlendState.BlendStates[0].DestinationAlpha);
            this->Blend.ColorOp = BlendOpToNative(PipelineCreation.BlendState.BlendStates[0].ColorOp);
            this->Blend.AlphaOp = BlendOpToNative(PipelineCreation.BlendState.BlendStates[0].AlphaOp);
            this->Blend.Separate = PipelineCreation.BlendState.BlendStates[0].SeparateBlend;
        }

        //Rasterizer state
        this->Rasterizer.CullMode = CullModeToNative(PipelineCreation.Rasterization.CullMode);
        this->Rasterizer.FillMode = FillModeToNative(PipelineCreation.Rasterization.Fill);
        this->Rasterizer.FrontFace = FrontFaceToNative(PipelineCreation.Rasterization.FrontFace);
    }

    for (size_t j = 0; j < PipelineCreation.Shaders.StagesCount; j++)
    {
        DeallocateMemory((void*)PipelineCreation.Shaders.Stages[j].Code);
        DeallocateMemory((void*)PipelineCreation.Shaders.Stages[j].FileName);
    }
    DeallocateMemory((void*)PipelineCreation.Name);    
}

void glPipeline::DestroyGLResources()
{
    if(this->ShaderProgram->VertexShader != nullptr) glDeleteShader(this->ShaderProgram->VertexShader->ShaderObject);
    if(this->ShaderProgram->GeometryShader != nullptr) glDeleteShader(this->ShaderProgram->GeometryShader->ShaderObject);
    if(this->ShaderProgram->FragmentShader != nullptr) glDeleteShader(this->ShaderProgram->FragmentShader->ShaderObject);
    if(this->ShaderProgram->ComputeShader != nullptr) glDeleteShader(this->ShaderProgram->ComputeShader->ShaderObject);
    glDeleteProgram(this->ShaderProgram->ProgramShaderObject);
}


}