#include "Include/Material.h"
#include "Include/Util.h"

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

unlitMaterial::~unlitMaterial()  
{
    gfx::context::Get()->WaitIdle();
    gfx::context::Get()->DestroyBuffer(this->UniformBuffer);
    gfx::context::Get()->DestroyPipeline(this->PipelineHandle);
}

std::vector<u8> unlitMaterial::Serialize() 
{
    std::vector<u8> Result;
    u32 MaterialType = (u32)materialType::Unlit;
    AddItem(Result, &MaterialType, sizeof(u32));
    AddItem(Result, &this->UniformData.Color, sizeof(v4f));
    return Result;
}

}