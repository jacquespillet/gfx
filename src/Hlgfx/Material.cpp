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
std::shared_ptr<texture> defaultTextures::PurpleTexture = nullptr;
std::shared_ptr<texture> defaultTextures::BlackTexture = nullptr;
std::shared_ptr<texture> defaultTextures::BlueTexture = nullptr;
std::shared_ptr<texture> defaultTextures::WhiteTexture = nullptr;

material::material(std::string Name)
{
    this->Name = Name;
    this->ID = context::Get()->Project.Materials.size();
}

customMaterial::customMaterial(std::string Name, gfx::pipelineHandle Pipeline) : material(Name)
{
    this->PipelineHandle = Pipeline;
}
void customMaterial::DrawGUI(){}
void customMaterial::SetCullMode(gfx::cullMode Mode) {}
void customMaterial::Serialize(const std::string &FileName){}
void customMaterial::RecreatePipeline() {}
std::shared_ptr<material> customMaterial::Clone() {
    std::shared_ptr<customMaterial> Result = std::make_shared<customMaterial>(this->Name + "_Duplicated", this->PipelineHandle);
    return Result;    
}
customMaterial::~customMaterial()
{
    printf("Destroying Material \n");
    if(this->UniformBuffer != gfx::InvalidHandle)
        gfx::context::Get()->QueueDestroyBuffer(this->UniformBuffer);
}

pbrMaterial::pbrMaterial(std::string Name) : material(Name)
{
    gfx::context *Context = gfx::context::Get();
    Flags =  (materialFlags::bits)(materialFlags::PBR | materialFlags::BlendEnabled | materialFlags::CullModeOn | materialFlags::DepthWriteEnabled | materialFlags::DepthTestEnabled);
    context::Get()->SetRenderFlags(Flags);
    
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
    this->UniformData.UseNormalTexture = 1.0f;
    this->UniformData.UseMetallicRoughnessTexture = 1.0f;

    Context->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(materialData), 0);
}

pbrMaterial::pbrMaterial(std::string Name, materialFlags::bits Flags) : material(Name)
{
    gfx::context *Context = gfx::context::Get();
    context::Get()->SetRenderFlags(Flags);


    this->BaseColorTexture = defaultTextures::WhiteTexture;
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
    this->UniformData.UseNormalTexture = 1.0f;
    this->UniformData.UseMetallicRoughnessTexture = 1.0f;


    Context->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(materialData), 0);
}
  
void pbrMaterial::SetCullMode(gfx::cullMode Mode)
{
    //Rebuild pipeline and replace the handle
}

void pbrMaterial::SetBaseColorTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->BaseColorTexture->ID);
    this->BaseColorTexture = Texture;
    this->UniformData.ColourTextureID = Texture->ID;
    this->Uniforms->Uniforms[1].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();
    this->AllTextures[this->BaseColorTexture->ID] = Texture;
}

void pbrMaterial::SetMetallicRoughnessTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->MetallicRoughnessTexture->ID);
    this->MetallicRoughnessTexture = Texture;
    this->Uniforms->Uniforms[2].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();    
    this->AllTextures[this->MetallicRoughnessTexture->ID] = Texture;
}

void pbrMaterial::SetOcclusionTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->OcclusionTexture->ID);
    this->OcclusionTexture = Texture;
    this->Uniforms->Uniforms[3].ResourceHandle = Texture->Handle;
    this->Uniforms->Update(); 
    this->AllTextures[this->OcclusionTexture->ID] = Texture;
}

void pbrMaterial::SetNormalTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->NormalTexture->ID);
    this->NormalTexture = Texture;
    this->Uniforms->Uniforms[4].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();
    this->AllTextures[this->NormalTexture->ID] = Texture;
}

void pbrMaterial::SetEmissiveTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->EmissiveTexture->ID);
    this->EmissiveTexture = Texture;
    this->Uniforms->Uniforms[5].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();
    this->AllTextures[this->EmissiveTexture->ID] = Texture;
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
        std::vector<std::shared_ptr<material>> &Materials = context::Get()->Project.Materials;
        for (auto &Material : Materials)
        {
            if(Material->PipelineHandle == this->PipelineHandle) Uses++;
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
    Result->ID = context::Get()->Project.Textures.size();
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

    FileStream.write((char*)&this->ID, sizeof(u32));

    u32 NameSize = this->Name.size();
    FileStream.write((char*)&NameSize, sizeof(u32));
    FileStream.write(this->Name.data(), this->Name.size());
    
    FileStream.write((char*)&Flags, sizeof(u32));

    FileStream.write((char*)&this->UniformData, sizeof(materialData));

    u8 HasBaseColorTexture = u8(this->BaseColorTexture->Handle != defaultTextures::BlackTexture->Handle); 
    FileStream.write((char*)&HasBaseColorTexture, sizeof(u8));
    if(HasBaseColorTexture)
    {
        FileStream.write((char*)&this->BaseColorTexture->ID, sizeof(u32)); 
    }

    u8 HasMetallicRoughnessTexture = u8(this->MetallicRoughnessTexture->Handle != defaultTextures::BlackTexture->Handle); 
    FileStream.write((char*)&HasMetallicRoughnessTexture, sizeof(u8));
    if(HasMetallicRoughnessTexture)
    {
        FileStream.write((char*)&this->MetallicRoughnessTexture->ID, sizeof(u32)); 
    }

    u8 HasOcclusionTexture = u8(this->OcclusionTexture->Handle != defaultTextures::WhiteTexture->Handle && this->OcclusionTexture->Handle != defaultTextures::BlackTexture->Handle); 
    FileStream.write((char*)&HasOcclusionTexture, sizeof(u8));
    if(HasOcclusionTexture)
    {
        FileStream.write((char*)&this->OcclusionTexture->ID, sizeof(u32)); 
    }

    u8 HasNormalTexture = u8(this->NormalTexture->Handle != defaultTextures::BlueTexture->Handle && this->OcclusionTexture->Handle != defaultTextures::BlackTexture->Handle); 
    FileStream.write((char*)&HasNormalTexture, sizeof(u8));
    if(HasNormalTexture)
    {
        FileStream.write((char*)&this->NormalTexture->ID, sizeof(u32)); 
    }

    u8 HasEmissiveTexture = u8(this->EmissiveTexture->Handle != defaultTextures::BlackTexture->Handle); 
    FileStream.write((char*)&HasEmissiveTexture, sizeof(u8));
    if(HasEmissiveTexture)
    {
        FileStream.write((char*)&this->EmissiveTexture->ID, sizeof(u32)); 
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


    u32 ID;
    FileStream.read((char*)&ID, sizeof(u32));

    u32 NameSize;
    FileStream.read((char*)&NameSize, sizeof(u32));
    std::string Name; Name.resize(NameSize);
    FileStream.read(Name.data(), Name.size());

    u32 FlagsUI;
    FileStream.read((char*)&FlagsUI, sizeof(u32));

    materialFlags::bits Flags = (materialFlags::bits)FlagsUI;
    
    std::shared_ptr<pbrMaterial> Result = std::make_shared<pbrMaterial>(Name, Flags);
    Result->ID = ID;
    
    FileStream.read((char*)&Result->UniformData, sizeof(pbrMaterial::materialData));
    
    
    u8 HasBaseColorTexture;
    FileStream.read((char*)&HasBaseColorTexture, sizeof(u8));
    if(HasBaseColorTexture)
    {
        u32 ID;
        FileStream.read((char*)&ID, sizeof(u32));
        
        std::shared_ptr<texture> Texture = context::Get()->Project.Textures[ID];

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
        u32 ID;
        FileStream.read((char*)&ID, sizeof(u32));

        std::shared_ptr<texture> Texture = context::Get()->Project.Textures[ID];
        
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
        u32 ID;
        FileStream.read((char*)&ID, sizeof(u32));

        std::shared_ptr<texture> Texture = context::Get()->Project.Textures[ID];
        
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
        u32 ID;
        FileStream.read((char*)&ID, sizeof(u32));

        std::shared_ptr<texture> Texture = context::Get()->Project.Textures[ID];
        
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
        u32 ID;
        FileStream.read((char*)&ID, sizeof(u32));

        std::shared_ptr<texture> Texture = context::Get()->Project.Textures[ID];
        
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