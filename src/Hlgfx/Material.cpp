#include "Include/Material.h"
#include "Include/Util.h"
#include "Include/Bindings.h"
#include "Include/Context.h"

namespace hlgfx
{
gfx::imageHandle defaultTextures::BlackTexture = gfx::InvalidHandle;
gfx::imageHandle defaultTextures::BlueTexture = gfx::InvalidHandle;
gfx::imageHandle defaultTextures::WhiteTexture = gfx::InvalidHandle;

unlitMaterial::unlitMaterial()
{
    gfx::context *Context = gfx::context::Get();
    Flags =  (materialFlags::bits)(materialFlags::Unlit | materialFlags::BlendEnabled | materialFlags::CullModeOn);
    context::Get()->CreateOrGetPipeline(Flags);


    this->UniformBuffer = Context->CreateBuffer(sizeof(materialData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
    this->Uniforms = std::make_shared<gfx::uniformGroup>();
    this->Uniforms->Reset()
                  .AddUniformBuffer(MaterialDataBinding, this->UniformBuffer)
                  .AddTexture(UnlitDiffuseTextureBinding, defaultTextures::BlackTexture);

    Context->BindUniformsToPipeline(this->Uniforms, this->PipelineHandle, MaterialDescriptorSetBinding);
    Uniforms->Update();

    this->UniformData.Color = v4f(1,0,0,0);
    Context->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(materialData), 0);
}

unlitMaterial::unlitMaterial(materialFlags::bits Flags)
{
    gfx::context *Context = gfx::context::Get();

    gfx::pipelineHandle Pipeline = context::Get()->CreateOrGetPipeline(Flags);    
    this->PipelineHandle = Pipeline;
    this->UniformBuffer = Context->CreateBuffer(sizeof(materialData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
    this->Uniforms = std::make_shared<gfx::uniformGroup>();
    this->Uniforms->Reset()
                  .AddUniformBuffer(MaterialDataBinding, this->UniformBuffer)
                  .AddTexture(UnlitDiffuseTextureBinding, defaultTextures::BlackTexture);
    this->Flags = Flags;
    Context->BindUniformsToPipeline(this->Uniforms, this->PipelineHandle, MaterialDescriptorSetBinding);
    Uniforms->Update();

    this->UniformData.Color = v4f(1,0,0,0);
    Context->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(materialData), 0);
}
  
void unlitMaterial::SetCullMode(gfx::cullMode Mode)
{
    //Rebuild pipeline and replace the handle
}

void unlitMaterial::SetDiffuseTexture(std::shared_ptr<gfx::imageHandle> Texture)
{
    this->DiffuseTexture = Texture;
    this->Uniforms->Uniforms[1].ResourceHandle = *Texture.get();
    this->Uniforms->Update();
}

unlitMaterial::~unlitMaterial()  
{
    printf("Destroying Material \n");
    gfx::context::Get()->QueueDestroyBuffer(this->UniformBuffer);
    if (this->DiffuseTexture.use_count() == 1)
    {
        gfx::context::Get()->QueueDestroyImage(*this->DiffuseTexture.get());
    }
    this->DiffuseTexture = nullptr;
}

std::vector<u8> unlitMaterial::Serialize() 
{
    //TODO: Add images
    std::vector<u8> Result;
    u32 MaterialType = (u32)materialType::Unlit;
    AddItem(Result, &MaterialType, sizeof(u32));
    AddItem(Result, &Flags, sizeof(u32));
    AddItem(Result, &this->UniformData.Color, sizeof(v4f));

    u8 HasDiffuseTexture = u8(*this->DiffuseTexture.get() != defaultTextures::BlackTexture); 
    AddItem(Result, &HasDiffuseTexture, sizeof(u8));
    if(HasDiffuseTexture)
    {
        gfx::image *Image = gfx::context::Get()->GetImage(*this->DiffuseTexture.get());
        AddItem(Result, &Image->Extent.Width, sizeof(u32));
        AddItem(Result, &Image->Extent.Height, sizeof(u32));
        AddItem(Result, &Image->Format, sizeof(u32));
        AddItem(Result, &Image->ChannelCount , sizeof(u8));
        sz Size = Image->Data.size();
        AddItem(Result, &Size, sizeof(sz));
        AddItem(Result,  Image->Data.data(), Image->Data.size());
    }

    return Result;
}

}