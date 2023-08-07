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

unlitMaterial::unlitMaterial(std::string Name) : material(Name)
{
    gfx::context *Context = gfx::context::Get();
    Flags =  (materialFlags::bits)(materialFlags::Unlit | materialFlags::BlendEnabled | materialFlags::CullModeOn | materialFlags::DepthWriteEnabled | materialFlags::DepthTestEnabled);
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

unlitMaterial::unlitMaterial(std::string Name, materialFlags::bits Flags) : material(Name)
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
  
void unlitMaterial::SetCullMode(gfx::cullMode Mode)
{
    //Rebuild pipeline and replace the handle
}

void unlitMaterial::SetBaseColorTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->BaseColorTexture->UUID);
    this->BaseColorTexture = Texture;
    this->Uniforms->Uniforms[1].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();
    this->AllTextures[this->BaseColorTexture->UUID] = Texture;
}

void unlitMaterial::SetMetallicRoughnessTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->MetallicRoughnessTexture->UUID);
    this->MetallicRoughnessTexture = Texture;
    this->Uniforms->Uniforms[2].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();    
    this->AllTextures[this->MetallicRoughnessTexture->UUID] = Texture;
}

void unlitMaterial::SetOcclusionTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->OcclusionTexture->UUID);
    this->OcclusionTexture = Texture;
    this->Uniforms->Uniforms[3].ResourceHandle = Texture->Handle;
    this->Uniforms->Update(); 
    this->AllTextures[this->OcclusionTexture->UUID] = Texture;
}

void unlitMaterial::SetNormalTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->NormalTexture->UUID);
    this->NormalTexture = Texture;
    this->Uniforms->Uniforms[4].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();
    this->AllTextures[this->NormalTexture->UUID] = Texture;
}

void unlitMaterial::SetEmissiveTexture(std::shared_ptr<texture> Texture)
{
    this->AllTextures.erase(this->EmissiveTexture->UUID);
    this->EmissiveTexture = Texture;
    this->Uniforms->Uniforms[5].ResourceHandle = Texture->Handle;
    this->Uniforms->Update();
    this->AllTextures[this->EmissiveTexture->UUID] = Texture;
}

void unlitMaterial::RecreatePipeline()
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


b8 unlitMaterial::ShowTextureSelection(const char *ID, std::shared_ptr<texture> &Texture)
{
    const f32 ImageWidth = 50;
    const f32 ImageHeight = 30;
    // f32 Offset = ImageHeight / 2 - TextHeight/2;

    bool Changed=false;
    if(ImGui::BeginPopupModal(ID))
    {
        context::project &Project = context::Get()->Project;
        for (auto &Tex : Project.Textures)
        {
            ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
            if(SelectedTexture.get() == Tex.second.get()) Flags |= ImGuiTreeNodeFlags_Selected;
            
            gfx::image *Image = gfx::context::Get()->GetImage(Tex.second->Handle);
            ImGui::Image(Image->GetImGuiID(), ImVec2(ImageWidth, ImageHeight));
            ImGui::SameLine();
            ImGui::TreeNodeEx(Tex.second->Name.c_str(), Flags);
            if(ImGui::IsItemClicked())
            {
                SelectedTexture = Tex.second;
            }
            ImGui::TreePop();
        }
        
        if (ImGui::Button("Select"))
        {
            Texture = SelectedTexture;
            ImGui::CloseCurrentPopup();
            Changed = true;
        }
        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return Changed;
}

bool unlitMaterial::DrawTexture(const char *Name, std::shared_ptr<texture> &Texture, f32 &Use)
{   
    b8 Changed=false;
    f32 TextHeight = ImGui::CalcTextSize(Name).y;
    const f32 ImageWidth = 50;
    const f32 ImageHeight = 30;
    f32 Offset = ImageHeight / 2 - TextHeight/2;

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + Offset);
    b8 UseBool = Use > 0.0f; 
    if(ImGui::Checkbox(Name, &UseBool))
    {
        Use = UseBool ? 1.0f : 0.0f;
        Changed = true;
    }

    ImGui::SameLine();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - Offset);
    gfx::image *Image = gfx::context::Get()->GetImage(Texture->Handle);
    ImGui::Image(Image->GetImGuiID(), ImVec2(ImageWidth, ImageHeight));
    if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    if(ImGui::IsItemClicked())
    {
        ImGui::OpenPopup(Name);
    }
    Changed |= ShowTextureSelection(Name, Texture);            
 
    ImGui::SameLine();
    gfx::image *BlackImage = gfx::context::Get()->GetImage(defaultTextures::BlackTexture->Handle);
    ImGui::Image(BlackImage->GetImGuiID(), ImVec2(30, ImageHeight));
    if(ImGui::IsItemClicked())
    {
        Changed = true;
        Texture = defaultTextures::BlackTexture;
    }

    ImGui::SameLine();
    gfx::image *WhiteImage = gfx::context::Get()->GetImage(defaultTextures::WhiteTexture->Handle);
    ImGui::Image(WhiteImage->GetImGuiID(), ImVec2(30, ImageHeight));
    if(ImGui::IsItemClicked())
    {
        Changed = true;
        Texture = defaultTextures::WhiteTexture;
    }

    ImGui::SameLine();
    gfx::image *BlueImage = gfx::context::Get()->GetImage(defaultTextures::BlueTexture->Handle);
    ImGui::Image(BlueImage->GetImGuiID(), ImVec2(30, ImageHeight));
    if(ImGui::IsItemClicked())
    {
        Changed = true;
        Texture = defaultTextures::BlueTexture;
    }

    
    
    return Changed;
}

std::shared_ptr<material> unlitMaterial::Clone()
{
    std::shared_ptr<unlitMaterial> Result = std::make_shared<unlitMaterial>(this->Name + "_Duplicated", this->Flags);
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

void unlitMaterial::DrawGUI()
{
    ImGui::Separator();
    ImGui::Text(this->Name.c_str());
    bool ShouldUpdate = false;
    bool ShouldRecreatePipeline = false;
    ShouldUpdate |= ImGui::ColorEdit3("Base Color", glm::value_ptr(this->UniformData.BaseColorFactor));
    ShouldUpdate |= ImGui::ColorEdit3("Emission", glm::value_ptr(this->UniformData.Emission));
    ShouldUpdate |= ImGui::DragFloat("Opacity", &this->UniformData.OpacityFactor, 0.005f, 0, 1);
    ShouldUpdate |= ImGui::DragFloat("Occlusion Strength", &this->UniformData.OcclusionStrength, 0.005f, 0, 1);
    ShouldUpdate |= ImGui::DragFloat("Emission Strength", &this->UniformData.EmissiveFactor, 0.005f, 0, 1);
    

    if(DrawTexture("Base Color Texture", this->BaseColorTexture, this->UniformData.UseBaseColor))
    {
        SetBaseColorTexture(this->BaseColorTexture);
        ShouldUpdate=true;
    }

    if(DrawTexture("Occlusion Texture", this->OcclusionTexture, this->UniformData.UseOcclusionTexture))
    {
        SetOcclusionTexture(this->OcclusionTexture);
        ShouldUpdate=true;
    }

    if(DrawTexture("Emissive Texture", this->EmissiveTexture, this->UniformData.UseEmissionTexture))
    {
        SetEmissiveTexture(this->EmissiveTexture);
        ShouldUpdate=true;
    }

    bool DepthWriteEnabled = this->Flags & materialFlags::DepthWriteEnabled;
    if(ImGui::Checkbox("Depth Write", &DepthWriteEnabled))
    {
        Flags = (materialFlags::bits)(Flags ^ materialFlags::DepthWriteEnabled);
        this->ShouldRecreate = true;
    }

    bool DepthTestEnabled = this->Flags & materialFlags::DepthTestEnabled;
    if(ImGui::Checkbox("Depth Test", &DepthTestEnabled))
    {
        Flags = (materialFlags::bits)(Flags ^ materialFlags::DepthTestEnabled);
        this->ShouldRecreate = true;
    }

    bool DoubleSided = !(this->Flags & materialFlags::CullModeOn);
    if(ImGui::Checkbox("Double Sided", &DoubleSided))
    {
        Flags = (materialFlags::bits)(Flags ^ materialFlags::CullModeOn);
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

void unlitMaterial::Serialize(const std::string &FileName) 
{
    std::ofstream FileStream;
    FileStream.open(FileName, std::ios::trunc | std::ios::binary);
    assert(FileStream.is_open());

    u32 MaterialType = (u32)materialType::Unlit;
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

    u8 HasOcclusionTexture = u8(this->OcclusionTexture->Handle != defaultTextures::BlackTexture->Handle); 
    FileStream.write((char*)&HasOcclusionTexture, sizeof(u8));
    if(HasOcclusionTexture)
    {
        u32 UUIDSize = this->OcclusionTexture->UUID.size();
        FileStream.write((char*)&UUIDSize, sizeof(u32));
        FileStream.write(this->OcclusionTexture->UUID.data(), this->OcclusionTexture->UUID.size()); 
    }

    u8 HasNormalTexture = u8(this->NormalTexture->Handle != defaultTextures::BlackTexture->Handle); 
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

    std::shared_ptr<unlitMaterial> Result = std::make_shared<unlitMaterial>(Name, (materialFlags::bits)Flags);
    Result->UUID = UUID;
    
    FileStream.read((char*)&Result->UniformData, sizeof(unlitMaterial::materialData));
    
    
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

void unlitMaterial::Update()
{
    gfx::context::Get()->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(materialData), 0);
}

}