#include "Include/Material.h"
#include "Include/Util.h"
#include "Include/Bindings.h"
#include "Include/Context.h"
#include "Include/Scene.h"
#include "Include/Mesh.h"

#include <glm/ext.hpp>
#include <nfd.h>
#include <fstream>

namespace hlgfx
{
std::shared_ptr<texture> defaultTextures::BlackTexture = std::make_shared<texture>("Black", gfx::InvalidHandle);
std::shared_ptr<texture> defaultTextures::BlueTexture = std::make_shared<texture>("Blue", gfx::InvalidHandle);
std::shared_ptr<texture> defaultTextures::WhiteTexture = std::make_shared<texture>("White", gfx::InvalidHandle);

material::material(std::string Name)
{
    this->Name = Name;
    this->UUID = context::Get()->GetUUID();
}

pbrMaterial::pbrMaterial(std::string Name) : material(Name)
{
    gfx::context *Context = gfx::context::Get();
    Flags =  (materialFlags::bits)(materialFlags::PBR | materialFlags::BlendEnabled | materialFlags::CullModeOn | materialFlags::DepthWriteEnabled | materialFlags::DepthTestEnabled);
    this->PipelineHandle = context::Get()->CreateOrGetPipeline(Flags);

    this->BaseColorTexture = defaultTextures::BlackTexture;
    this->NormalTexture = defaultTextures::BlueTexture;
    this->EmissiveTexture = defaultTextures::BlackTexture;
    this->OcclusionTexture = defaultTextures::WhiteTexture;
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
    this->UniformData.BaseColorFactor = v3f(1,1,1);
    this->UniformData.OpacityFactor = 1.0f;
    this->UniformData.DebugChannel = -1;
    this->UniformData.OcclusionStrength = 1;
    this->UniformData.Emission = v3f(0,0,0);
    this->UniformData.UseBaseColor = 1.0f;
    this->UniformData.UseEmissionTexture = 1.0f;
    this->UniformData.UseOcclusionTexture = 1.0f;

    Context->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(materialData), 0);
}

pbrMaterial::pbrMaterial(std::string Name, materialFlags::bits Flags) : material(Name)
{
    gfx::context *Context = gfx::context::Get();

    this->BaseColorTexture = defaultTextures::BlackTexture;
    this->NormalTexture = defaultTextures::BlueTexture;
    this->EmissiveTexture = defaultTextures::BlackTexture;
    this->OcclusionTexture = defaultTextures::WhiteTexture;
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
    this->UniformData.OcclusionStrength = 1;
    this->UniformData.Emission = v3f(0,0,0);
    this->UniformData.UseBaseColor = 1.0f;
    this->UniformData.UseEmissionTexture = 1.0f;
    this->UniformData.UseOcclusionTexture = 1.0f;

    Context->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(materialData), 0);
}
  
void pbrMaterial::SetCullMode(gfx::cullMode Mode)
{
    //Rebuild pipeline and replace the handle
}

void pbrMaterial::SetBaseColorTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->BaseColorTexture->UUID);
    this->BaseColorTexture = Texture;
    this->Uniforms->Uniforms[1].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();
    this->AllTextures[this->BaseColorTexture->UUID] = Texture;
}

void pbrMaterial::SetMetallicRoughnessTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->MetallicRoughnessTexture->UUID);
    this->MetallicRoughnessTexture = Texture;
    this->Uniforms->Uniforms[2].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();    
    this->AllTextures[this->MetallicRoughnessTexture->UUID] = Texture;
}

void pbrMaterial::SetOcclusionTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->OcclusionTexture->UUID);
    this->OcclusionTexture = Texture;
    this->Uniforms->Uniforms[3].ResourceHandle = Texture->Handle;
    this->Uniforms->Update(); 
    this->AllTextures[this->OcclusionTexture->UUID] = Texture;
}

void pbrMaterial::SetNormalTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->NormalTexture->UUID);
    this->NormalTexture = Texture;
    this->Uniforms->Uniforms[4].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();
    this->AllTextures[this->NormalTexture->UUID] = Texture;
}

void pbrMaterial::SetEmissiveTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->EmissiveTexture->UUID);
    this->EmissiveTexture = Texture;
    this->Uniforms->Uniforms[5].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();
    this->AllTextures[this->EmissiveTexture->UUID] = Texture;
}

void pbrMaterial::RecreatePipeline()
{

    //Get the pipeline if it exists already
    gfx::pipelineHandle Pipeline = context::Get()->GetPipeline(this->Flags);
    if(Pipeline != gfx::InvalidHandle)
    {
        this->PipelineHandle = Pipeline;
    }
    else
    {
        u32 Uses = 0;
        std::unordered_map<std::string, std::shared_ptr<material>> &Materials = context::Get()->Project.Materials;
        for (auto &Material : Materials)
        {
            if(Material.second->PipelineHandle == this->PipelineHandle) Uses++;
        }
        
        gfx::context::Get()->WaitIdle();
        if(Uses == 1)
        {
            //If this is the only material that uses this pipeline, we can delete it and recreate it
            gfx::pipelineCreation PipelineCreation = context::Get()->GetPipelineCreation(this->Flags);
            gfx::context::Get()->RecreatePipeline(PipelineCreation, this->PipelineHandle);
        }
        else
        {
            //If other materials use this pipeline, we cannot delete it. we need to create a new one
            this->PipelineHandle = context::Get()->CreateOrGetPipeline(this->Flags);
        }
    }
    gfx::context::Get()->BindUniformsToPipeline(this->Uniforms, this->PipelineHandle, MaterialDescriptorSetBinding, true);
    Uniforms->Update();
    this->ShouldRecreate=false;
}


std::shared_ptr<material> pbrMaterial::Clone()
{
    std::shared_ptr<pbrMaterial> Result = std::make_shared<pbrMaterial>(this->Name + "_Duplicated", this->Flags);
    Result->UniformData = this->UniformData;
    Result->UUID = context::Get()->GetUUID();
    Result->SetBaseColorTexture(this->BaseColorTexture);
    Result->SetOcclusionTexture(this->OcclusionTexture);
    Result->SetEmissiveTexture(this->EmissiveTexture);
    Result->SetMetallicRoughnessTexture(this->MetallicRoughnessTexture);
    Result->SetNormalTexture(this->NormalTexture);
    Result->Uniforms->Update();
    Result->Update();

    return Result;
}
  
pbrMaterial::~pbrMaterial()  
{
    printf("Destroying Material \n");
    gfx::context::Get()->QueueDestroyBuffer(this->UniformBuffer);
}

void pbrMaterial::Serialize(const std::string &FileName) 
{
    std::ofstream FileStream;
    FileStream.open(FileName, std::ios::trunc | std::ios::binary);
    assert(FileStream.is_open());

    u32 MaterialType = (u32)materialType::PBR;
    FileStream.write((char*)&MaterialType, sizeof(u32));

    u32 UUIDSize = this->UUID.size();
    FileStream.write((char*)&UUIDSize, sizeof(u32));
    FileStream.write(this->UUID.data(), this->UUID.size());

    u32 NameSize = this->Name.size();
    FileStream.write((char*)&NameSize, sizeof(u32));
    FileStream.write(this->Name.data(), this->Name.size());
    
    FileStream.write((char*)&Flags, sizeof(u32));

    FileStream.write((char*)&this->UniformData, sizeof(materialData));

    u8 HasBaseColorTexture = u8(this->BaseColorTexture->Handle != defaultTextures::BlackTexture->Handle); 
    FileStream.write((char*)&HasBaseColorTexture, sizeof(u8));
    if(HasBaseColorTexture)
    {
        u32 UUIDSize = this->BaseColorTexture->UUID.size();
        FileStream.write((char*)&UUIDSize, sizeof(u32));
        FileStream.write(this->BaseColorTexture->UUID.data(), this->BaseColorTexture->UUID.size()); 
    }

    u8 HasMetallicRoughnessTexture = u8(this->MetallicRoughnessTexture->Handle != defaultTextures::BlackTexture->Handle); 
    FileStream.write((char*)&HasMetallicRoughnessTexture, sizeof(u8));
    if(HasMetallicRoughnessTexture)
    {
        u32 UUIDSize = this->MetallicRoughnessTexture->UUID.size();
        FileStream.write((char*)&UUIDSize, sizeof(u32));
        FileStream.write(this->MetallicRoughnessTexture->UUID.data(), this->MetallicRoughnessTexture->UUID.size()); 
    }

    u8 HasOcclusionTexture = u8(this->OcclusionTexture->Handle != defaultTextures::WhiteTexture->Handle && this->OcclusionTexture->Handle != defaultTextures::BlackTexture->Handle); 
    FileStream.write((char*)&HasOcclusionTexture, sizeof(u8));
    if(HasOcclusionTexture)
    {
        u32 UUIDSize = this->OcclusionTexture->UUID.size();
        FileStream.write((char*)&UUIDSize, sizeof(u32));
        FileStream.write(this->OcclusionTexture->UUID.data(), this->OcclusionTexture->UUID.size()); 
    }

    u8 HasNormalTexture = u8(this->NormalTexture->Handle != defaultTextures::BlueTexture->Handle && this->OcclusionTexture->Handle != defaultTextures::BlackTexture->Handle); 
    FileStream.write((char*)&HasNormalTexture, sizeof(u8));
    if(HasNormalTexture)
    {
        u32 UUIDSize = this->NormalTexture->UUID.size();
        FileStream.write((char*)&UUIDSize, sizeof(u32));
        FileStream.write(this->NormalTexture->UUID.data(), this->NormalTexture->UUID.size()); 
    }

    u8 HasEmissiveTexture = u8(this->EmissiveTexture->Handle != defaultTextures::BlackTexture->Handle); 
    FileStream.write((char*)&HasEmissiveTexture, sizeof(u8));
    if(HasEmissiveTexture)
    {
        u32 UUIDSize = this->EmissiveTexture->UUID.size();
        FileStream.write((char*)&UUIDSize, sizeof(u32));
        FileStream.write(this->EmissiveTexture->UUID.data(), this->EmissiveTexture->UUID.size()); 
    }

    FileStream.close();
}

std::shared_ptr<material> material::Deserialize(const std::string &FileName)
{
    std::ifstream FileStream;
    FileStream.open(FileName, std::ios::binary);
    assert(FileStream.is_open());

    u32 MaterialType;
    FileStream.read((char*)&MaterialType, sizeof(u32));


    u32 UUIDSize;
    FileStream.read((char*)&UUIDSize, sizeof(u32));
    std::string UUID; UUID.resize(UUIDSize);
    FileStream.read(UUID.data(), UUID.size());

    u32 NameSize;
    FileStream.read((char*)&NameSize, sizeof(u32));
    std::string Name; Name.resize(NameSize);
    FileStream.read(Name.data(), Name.size());

    u32 Flags;
    FileStream.read((char*)&Flags, sizeof(u32));

    std::shared_ptr<pbrMaterial> Result = std::make_shared<pbrMaterial>(Name, (materialFlags::bits)Flags);
    Result->UUID = UUID;
    
    FileStream.read((char*)&Result->UniformData, sizeof(pbrMaterial::materialData));
    
    
    u8 HasBaseColorTexture;
    FileStream.read((char*)&HasBaseColorTexture, sizeof(u8));
    if(HasBaseColorTexture)
    {
        u32 UUIDSize;
        FileStream.read((char*)&UUIDSize, sizeof(u32));
        std::string UUID; UUID.resize(UUIDSize);
        FileStream.read(UUID.data(), UUID.size());

        std::shared_ptr<texture> Texture = context::Get()->Project.Textures[UUID];
        Result->SetBaseColorTexture(Texture); 
    }
    else
    {
        Result->SetBaseColorTexture(defaultTextures::BlackTexture);
    }
    
    u8 HasMetallicRoughnessTexture;
    FileStream.read((char*)&HasMetallicRoughnessTexture, sizeof(u8));
    if(HasMetallicRoughnessTexture)
    {
        u32 UUIDSize;
        FileStream.read((char*)&UUIDSize, sizeof(u32));
        std::string UUID; UUID.resize(UUIDSize);
        FileStream.read(UUID.data(), UUID.size());

        std::shared_ptr<texture> Texture = context::Get()->Project.Textures[UUID];
        Result->SetMetallicRoughnessTexture(Texture); 
    }
    else
    {
        Result->SetMetallicRoughnessTexture(defaultTextures::BlackTexture);
    }
    
    u8 HasOcclusionTexture;
    FileStream.read((char*)&HasOcclusionTexture, sizeof(u8));
    if(HasOcclusionTexture)
    {
        u32 UUIDSize;
        FileStream.read((char*)&UUIDSize, sizeof(u32));
        std::string UUID; UUID.resize(UUIDSize);
        FileStream.read(UUID.data(), UUID.size());

        std::shared_ptr<texture> Texture = context::Get()->Project.Textures[UUID];
        Result->SetOcclusionTexture(Texture); 
    }
    else
    {
        Result->SetOcclusionTexture(defaultTextures::WhiteTexture);
    }
    
    u8 HasNormalTexture;
    FileStream.read((char*)&HasNormalTexture, sizeof(u8));
    if(HasNormalTexture)
    {
        u32 UUIDSize;
        FileStream.read((char*)&UUIDSize, sizeof(u32));
        std::string UUID; UUID.resize(UUIDSize);
        FileStream.read(UUID.data(), UUID.size());

        std::shared_ptr<texture> Texture = context::Get()->Project.Textures[UUID];
        Result->SetNormalTexture(Texture); 
    }
    else
    {
        Result->SetNormalTexture(defaultTextures::BlueTexture);
    }
    
    u8 HasEmissiveTexture;
    FileStream.read((char*)&HasEmissiveTexture, sizeof(u8));
    if(HasEmissiveTexture)
    {
        u32 UUIDSize;
        FileStream.read((char*)&UUIDSize, sizeof(u32));
        std::string UUID; UUID.resize(UUIDSize);
        FileStream.read(UUID.data(), UUID.size());

        std::shared_ptr<texture> Texture = context::Get()->Project.Textures[UUID];
        Result->SetEmissiveTexture(Texture); 
    }
    else
    {
        Result->SetEmissiveTexture(defaultTextures::BlackTexture);
    }
    
    Result->Uniforms->Update();
    Result->Update();
    return Result;
}

void pbrMaterial::Update()
{
    gfx::context::Get()->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(materialData), 0);
}

}