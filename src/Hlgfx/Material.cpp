#include "Include/Material.h"
#include "Include/Util.h"
#include "Include/Bindings.h"
#include "Include/Context.h"

#include <glm/ext.hpp>

namespace hlgfx
{
std::shared_ptr<texture> defaultTextures::BlackTexture = std::make_shared<texture>(gfx::InvalidHandle);
std::shared_ptr<texture> defaultTextures::BlueTexture = std::make_shared<texture>(gfx::InvalidHandle);
std::shared_ptr<texture> defaultTextures::WhiteTexture = std::make_shared<texture>(gfx::InvalidHandle);

unlitMaterial::unlitMaterial()
{
    gfx::context *Context = gfx::context::Get();
    Flags =  (materialFlags::bits)(materialFlags::Unlit | materialFlags::BlendEnabled | materialFlags::CullModeOn | materialFlags::DepthWriteEnabled | materialFlags::DepthTestEnabled);
    context::Get()->CreateOrGetPipeline(Flags);

    this->BaseColorTexture = defaultTextures::BlackTexture;
    this->NormalTexture = defaultTextures::BlueTexture;
    this->EmissiveTexture = defaultTextures::BlackTexture;
    this->OcclusionTexture = defaultTextures::BlackTexture;
    this->MetallicRoughnessTexture = defaultTextures::BlackTexture;

    this->UniformBuffer = Context->CreateBuffer(sizeof(materialData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
    this->Uniforms = std::make_shared<gfx::uniformGroup>();
    this->Uniforms->Reset()
                  .AddUniformBuffer(MaterialDataBinding, this->UniformBuffer)
                  .AddTexture(BaseColorTextureBinding, this->BaseColorTexture->Handle)
                  .AddTexture(MetallicRoughnessTextureBinding, this->MetallicRoughnessTexture->Handle)
                  .AddTexture(OcclusionTextureBinding, this->OcclusionTexture->Handle)
                  .AddTexture(NormalTextureBinding, this->NormalTexture->Handle)
                  .AddTexture(EmissiveTextureBinding, this->EmissiveTexture->Handle);

    Context->BindUniformsToPipeline(this->Uniforms, this->PipelineHandle, MaterialDescriptorSetBinding);
    Uniforms->Update();

    this->UniformData.RoughnessFactor = 1.0f;
    this->UniformData.MetallicFactor = 0.0f;
    this->UniformData.EmissiveFactor = 0.0f;
    this->UniformData.AlphaCutoff = 0.0f;
    this->UniformData.BaseColorFactor = v3f(0,0,0);
    this->UniformData.OpacityFactor = 1.0f;
    this->UniformData.DebugChannel = -1;
    this->UniformData.OcclusionStrength = 0;
    this->UniformData.Emission = v3f(0,0,0);

    Context->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(materialData), 0);
}

unlitMaterial::unlitMaterial(materialFlags::bits Flags)
{
    gfx::context *Context = gfx::context::Get();

    this->BaseColorTexture = defaultTextures::BlackTexture;
    this->NormalTexture = defaultTextures::BlueTexture;
    this->EmissiveTexture = defaultTextures::BlackTexture;
    this->OcclusionTexture = defaultTextures::BlackTexture;
    this->MetallicRoughnessTexture = defaultTextures::BlackTexture;

    gfx::pipelineHandle Pipeline = context::Get()->CreateOrGetPipeline(Flags);    
    this->PipelineHandle = Pipeline;
    this->UniformBuffer = Context->CreateBuffer(sizeof(materialData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
    this->Flags = Flags;
    this->Uniforms = std::make_shared<gfx::uniformGroup>();
    this->Uniforms->Reset()
                  .AddUniformBuffer(MaterialDataBinding, this->UniformBuffer)
                  .AddTexture(BaseColorTextureBinding, this->BaseColorTexture->Handle)
                  .AddTexture(MetallicRoughnessTextureBinding, this->MetallicRoughnessTexture->Handle)
                  .AddTexture(OcclusionTextureBinding, this->OcclusionTexture->Handle)
                  .AddTexture(NormalTextureBinding, this->NormalTexture->Handle)
                  .AddTexture(EmissiveTextureBinding, this->EmissiveTexture->Handle);
    Context->BindUniformsToPipeline(this->Uniforms, this->PipelineHandle, MaterialDescriptorSetBinding);
    Uniforms->Update();

    this->UniformData.RoughnessFactor = 1.0f;
    this->UniformData.MetallicFactor = 0.0f;
    this->UniformData.EmissiveFactor = 0.0f;
    this->UniformData.AlphaCutoff = 0.0f;
    this->UniformData.BaseColorFactor = v3f(0,0,0);
    this->UniformData.OpacityFactor = 1.0f;
    this->UniformData.DebugChannel = -1;
    this->UniformData.OcclusionStrength = 0;
    this->UniformData.Emission = v3f(0,0,0);

    Context->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(materialData), 0);
}
  
void unlitMaterial::SetCullMode(gfx::cullMode Mode)
{
    //Rebuild pipeline and replace the handle
}

void unlitMaterial::SetDiffuseTexture(std::shared_ptr<texture> Texture)
{
    this->BaseColorTexture = Texture;
    this->Uniforms->Uniforms[1].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();
}

void unlitMaterial::SetMetallicRoughnessTexture(std::shared_ptr<texture> Texture)
{
    this->MetallicRoughnessTexture = Texture;
    this->Uniforms->Uniforms[2].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();    
}

void unlitMaterial::SetOcclusionTexture(std::shared_ptr<texture> Texture)
{
    this->OcclusionTexture = Texture;
    this->Uniforms->Uniforms[3].ResourceHandle = Texture->Handle;
    this->Uniforms->Update(); 
}

void unlitMaterial::SetNormalTexture(std::shared_ptr<texture> Texture)
{
    this->NormalTexture = Texture;
    this->Uniforms->Uniforms[4].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();
}

void unlitMaterial::SetEmissiveTexture(std::shared_ptr<texture> Texture)
{
    this->EmissiveTexture = Texture;
    this->Uniforms->Uniforms[5].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();
}

void unlitMaterial::RecreatePipeline()
{
    gfx::context::Get()->WaitIdle();
    gfx::pipelineCreation PipelineCreation = context::Get()->GetPipelineCreation(this->Flags);
    gfx::context::Get()->RecreatePipeline(PipelineCreation, this->PipelineHandle);
    
    
    gfx::context::Get()->BindUniformsToPipeline(this->Uniforms, this->PipelineHandle, MaterialDescriptorSetBinding, true);
    Uniforms->Update();
    this->ShouldRecreate=false;
}

void unlitMaterial::DrawGUI()
{
    bool ShouldUpdate = false;
    bool ShouldRecreatePipeline = false;
    ShouldUpdate |= ImGui::ColorEdit3("Base Color", glm::value_ptr(this->UniformData.BaseColorFactor));
    ShouldUpdate |= ImGui::DragFloat("Opacity", &this->UniformData.OpacityFactor, 0.001f, 0, 1);
    


    bool DepthWriteEnabled = this->Flags & materialFlags::DepthWriteEnabled;
    if(ImGui::Checkbox("Depth Write", &DepthWriteEnabled))
    {
        Flags = (materialFlags::bits)(Flags ^ materialFlags::DepthWriteEnabled);
        this->ShouldRecreate = true;
    }

    bool BlendEnabled = this->Flags & materialFlags::BlendEnabled;
    if(ImGui::Checkbox("Transparent", &BlendEnabled))
    {
        Flags = (materialFlags::bits)(Flags ^ materialFlags::BlendEnabled);
        this->ShouldRecreate = true;
    }


    if(ShouldUpdate)
        Update();
}

unlitMaterial::~unlitMaterial()  
{
    printf("Destroying Material \n");
    gfx::context::Get()->QueueDestroyBuffer(this->UniformBuffer);
}

std::vector<u8> unlitMaterial::Serialize() 
{
    std::vector<u8> Result;
    u32 MaterialType = (u32)materialType::Unlit;
    AddItem(Result, &MaterialType, sizeof(u32));
    AddItem(Result, &Flags, sizeof(u32));
    AddItem(Result, &this->UniformData.BaseColorFactor, sizeof(v4f));

    u8 HasDiffuseTexture = u8(this->BaseColorTexture->Handle != defaultTextures::BlackTexture->Handle); 
    AddItem(Result, &HasDiffuseTexture, sizeof(u8));
    if(HasDiffuseTexture)
    {
        gfx::image *Image = gfx::context::Get()->GetImage(this->BaseColorTexture->Handle);
        AddItem(Result, &Image->Extent.Width, sizeof(u32));
        AddItem(Result, &Image->Extent.Height, sizeof(u32));
        AddItem(Result, &Image->Format, sizeof(u32));
        AddItem(Result, &Image->ChannelCount , sizeof(u8));
        AddItem(Result, &Image->Type , sizeof(gfx::type));
        sz Size = Image->Data.size();  
        AddItem(Result, &Size, sizeof(sz));
        AddItem(Result,  Image->Data.data(), Image->Data.size());
    }

    return Result;
}

void unlitMaterial::Update()
{
    gfx::context::Get()->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(materialData), 0);
}

}