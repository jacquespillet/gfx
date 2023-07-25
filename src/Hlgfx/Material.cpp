#include "Include/Material.h"

namespace hlgfx
{

unlitMaterial::unlitMaterial()
{
    gfx::context *Context = gfx::context::Get();

    PipelineHandle = Context->CreatePipelineFromFile("resources/Hlgfx/Shaders/Unlit/Unlit.json");

    this->UniformBuffer = Context->CreateBuffer(sizeof(materialData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
    this->Uniforms = std::make_shared<gfx::uniformGroup>();
    this->Uniforms->Reset().AddUniformBuffer(MaterialUniformsBinding, this->UniformBuffer);

    Context->BindUniformsToPipeline(this->Uniforms, this->PipelineHandle, MaterialUniformsBinding);
    Uniforms->Update();

    this->UniformData.Color = v4f(1,0,0,0);
    Context->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(materialData), 0);

    // PipelineHandle = context::Get()->Pipelines[context::UnlitPipeline];
}
  
void unlitMaterial::SetCullMode(gfx::cullMode Mode)
{
    //Rebuild pipeline and replace the handle
}

}