#include "Include/Mesh.h"
#include "Include/Material.h"
#include "Include/Context.h"
#include "Include/Scene.h"
#include "Include/Util.h"
#include "Include/Bindings.h"
#include "gfx/Include/Context.h"
#include <glm/ext.hpp>

namespace hlgfx
{
mesh::mesh(std::string Name) : object3D(Name)
{
    this->Material = nullptr;
    this->GeometryBuffers = nullptr;
    this->UUID = context::Get()->GetUUID();
    this->UniformBuffer = gfx::context::Get()->CreateBuffer(sizeof(uniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
    this->Uniforms = std::make_shared<gfx::uniformGroup>();
    this->Uniforms->Reset().AddUniformBuffer(ModelBinding, this->UniformBuffer);

    //Bind the uniform group to the context pipelines
    context *HighLevelContext = context::Get();
    gfx::context *LowLevelContext = gfx::context::Get();
    for(auto &Pipeline : HighLevelContext->Pipelines)
    {
        //This effectively allocates the descriptor sets based on the DS layouts of each pipeline.
        LowLevelContext->BindUniformsToPipeline(this->Uniforms, Pipeline.second, ModelDescriptorSetBinding);
    }
    
    this->UniformData.ModelMatrix = this->Transform.Matrices.LocalToWorld;
    gfx::context::Get()->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(uniformData), 0);

    this->Uniforms->Update(); 
}

mesh::mesh() : object3D("Mesh")
{
    this->Material = nullptr;
    this->GeometryBuffers = nullptr;

    this->UUID = context::Get()->GetUUID();
    this->UniformBuffer = gfx::context::Get()->CreateBuffer(sizeof(uniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
    this->Uniforms = std::make_shared<gfx::uniformGroup>();
    this->Uniforms->Reset().AddUniformBuffer(ModelBinding, this->UniformBuffer);

    //Bind the uniform group to the context pipelines
    context *HighLevelContext = context::Get();
    gfx::context *LowLevelContext = gfx::context::Get();
    for(auto &Pipeline : HighLevelContext->Pipelines)
    {
        //This effectively allocates the descriptor sets based on the DS layouts of each pipeline.
        LowLevelContext->BindUniformsToPipeline(this->Uniforms, Pipeline.second, ModelDescriptorSetBinding);
    }
    
    this->UniformData.ModelMatrix = this->Transform.Matrices.LocalToWorld;
    gfx::context::Get()->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(uniformData), 0);

    this->Uniforms->Update();    
}


std::shared_ptr<object3D> mesh::Clone(b8 CloneUUID)
{
    std::shared_ptr<mesh> Result = std::make_shared<mesh>(this->Name);
    
    Result->RenderOrder=this->RenderOrder;
    Result->FrustumCulled=this->FrustumCulled;
    Result->CastShadow=this->CastShadow;
    Result->ReceiveShadow=this->ReceiveShadow;
    if(CloneUUID) Result->UUID= this->UUID;
    Result->Transform =  this->Transform;
    Result->Transform.HasChanged=true;

    Result->UniformData = this->UniformData; 
    Result->GeometryBuffers = this->GeometryBuffers;
    Result->Material = this->Material;   

    
    b8 DoCompute = transform::DoCompute;
    transform::DoCompute=false;
    for (sz i = 0; i < this->Children.size(); i++)
    {
        Result->AddObject(this->Children[i]->Clone(CloneUUID));
    }
    if(DoCompute) transform::DoCompute=true;
    
    return Result;
}


void mesh::OnEarlyUpdate()
{
    if(this->Material->ShouldRecreate)
    {
        gfx::pipelineHandle OldPipeline = this->Material->PipelineHandle;
        this->Material->RecreatePipeline();
        if(OldPipeline != this->Material->PipelineHandle)
            context::Get()->Scene->UpdateMeshPipeline(OldPipeline, this);
    }
}


void mesh::OnRender(std::shared_ptr<camera> Camera) 
{
    if(this->Transform.HasChanged)
    {
        this->UniformData.ModelMatrix = this->Transform.Matrices.LocalToWorld;
        gfx::context::Get()->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(uniformData), 0);
    }
    std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();    
    CommandBuffer->BindUniformGroup(this->Uniforms, ModelDescriptorSetBinding);
    CommandBuffer->BindUniformGroup(this->Material->Uniforms, MaterialDescriptorSetBinding);
    
    CommandBuffer->BindVertexBuffer(this->GeometryBuffers->VertexBuffer);
    CommandBuffer->BindIndexBuffer(this->GeometryBuffers->IndexBuffer, this->GeometryBuffers->Start, gfx::indexType::Uint32);
    CommandBuffer->DrawIndexed(this->GeometryBuffers->Start, this->GeometryBuffers->Count, 1);
}

void mesh::ShowMaterialSelection(std::shared_ptr<material> &Material)
{
    if(ImGui::BeginPopupModal("Material Selection"))
    {
        context::project &Project = context::Get()->Project;
        for (auto &Material : Project.Materials)
        {
            ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
            if(SelectedMaterial.get() == Material.second.get()) Flags |= ImGuiTreeNodeFlags_Selected;
            ImGui::TreeNodeEx(Material.second->Name.c_str(), Flags);
            if(ImGui::IsItemClicked())
            {
                SelectedMaterial = Material.second;
            }
            ImGui::TreePop();
        }
        
        if (ImGui::Button("Select"))
        {
            gfx::pipelineHandle OldPipeline = Material->PipelineHandle;
            Material = SelectedMaterial;
            context::Get()->Scene->UpdateMeshPipeline(OldPipeline, this);
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void mesh::DrawMaterial()
{
    if(ImGui::Button("Set material"))
    {
        ImGui::OpenPopup("Material Selection");
    }
    ShowMaterialSelection(this->Material);
    
    this->Material->DrawGUI();
}

void mesh::Serialize(std::ofstream &FileStream)
{
    std::vector<u8> Result;

    u32 Object3DType = (u32) object3DType::Mesh;
    FileStream.write((char*)&Object3DType, sizeof(u32));

    u32 UUIDSize = this->UUID.size();
    FileStream.write((char*)&UUIDSize, sizeof(u32));
    FileStream.write(this->UUID.data(), this->UUID.size());
    
    u32 StringLength = this->Name.size();
    FileStream.write((char*)&StringLength, sizeof(u32));
    FileStream.write((char*)(void*)this->Name.data(), StringLength);
    
    FileStream.write((char*)&this->Transform.Matrices, sizeof(transform::matrices));
    FileStream.write((char*)&this->Transform.LocalValues, sizeof(transform::localValues));
    
    u32 GeometryUUIDSize = this->GeometryBuffers->UUID.size();
    FileStream.write((char*)&GeometryUUIDSize, sizeof(u32));
    FileStream.write(this->GeometryBuffers->UUID.data(), GeometryUUIDSize);
    
    u32 MaterialUUIDSize = this->Material->UUID.size();
    FileStream.write((char*)&MaterialUUIDSize, sizeof(u32));
    FileStream.write(this->Material->UUID.data(), MaterialUUIDSize);


    u32 NumChildren = this->Children.size();
    FileStream.write((char*)&NumChildren, sizeof(u32));

    for (sz i = 0; i < NumChildren; i++)
    {
        this->Children[i]->Serialize(FileStream);
    }
}

mesh::~mesh()
{
    printf("Destroying Mesh\n");
    gfx::context::Get()->QueueDestroyBuffer(this->UniformBuffer);
    if(this->GeometryBuffers.use_count() == 1)
    {
        gfx::context::Get()->QueueDestroyBuffer(this->GeometryBuffers->IndexBuffer);
        gfx::context::Get()->QueueDestroyVertexBuffer(this->GeometryBuffers->VertexBuffer);
    }
}


}