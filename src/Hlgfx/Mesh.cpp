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
    this->MaterialID = (u32)-1;
    this->GeometryID = (u32)-1;
    this->ID = context::Get()->Project.Objects.size();
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
    this->UniformData.NormalMatrix = this->Transform.Matrices.LocalToWorldNormal;
    gfx::context::Get()->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(uniformData), 0);

    this->Uniforms->Update(); 
}

mesh::mesh() : object3D("Mesh")
{
    this->MaterialID = (u32)-1;
    this->GeometryID = (u32)-1;

    this->ID = context::Get()->Project.Objects.size();
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
    this->UniformData.NormalMatrix = this->Transform.Matrices.LocalToWorldNormal;
    gfx::context::Get()->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(uniformData), 0);

    this->Uniforms->Update();    
}


std::shared_ptr<object3D> mesh::Clone(b8 CloneID)
{
    std::shared_ptr<mesh> Result = std::make_shared<mesh>(this->Name);
    
    Result->RenderOrder=this->RenderOrder;
    Result->FrustumCulled=this->FrustumCulled;
    Result->CastShadow=this->CastShadow;
    Result->ReceiveShadow=this->ReceiveShadow;
    if(CloneID) Result->ID= this->ID;
    Result->Transform =  this->Transform;
    Result->Transform.HasChanged=true;

    Result->UniformData = this->UniformData; 
    Result->GeometryID = this->GeometryID;
    Result->MaterialID = this->MaterialID;   

    
    b8 DoCompute = transform::DoCompute;
    transform::DoCompute=false;
    for (sz i = 0; i < this->Children.size(); i++)
    {
        Result->AddObject(this->Children[i]->Clone(CloneID));
    }
    if(DoCompute) transform::DoCompute=true;
    
    return Result;
}


void mesh::OnEarlyUpdate()
{
    std::shared_ptr<material> Material = context::Get()->Project.Materials[this->MaterialID];
    if(Material->ShouldRecreate)
    {
        gfx::pipelineHandle OldPipeline = Material->PipelineHandle;
        Material->RecreatePipeline();
        if(OldPipeline != Material->PipelineHandle)
            context::Get()->Scene->UpdateMeshPipeline(OldPipeline, this);
    }
    if(Material->ShouldUpdateUniforms)
    {
        Material->Uniforms->Update();
        Material->ShouldUpdateUniforms=false;
    }
}


void mesh::OnRender(std::shared_ptr<camera> Camera) 
{
    if(this->Transform.HasChanged)
    {
        this->UniformData.ModelMatrix = this->Transform.Matrices.LocalToWorld;
        this->UniformData.NormalMatrix = this->Transform.Matrices.LocalToWorldNormal;
        gfx::context::Get()->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(uniformData), 0);
        
        context::Get()->Scene->UpdateBLASInstance(this->MeshSceneID);
        this->Transform.HasChanged=false;
    }
    std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();    
    CommandBuffer->BindUniformGroup(this->Uniforms, ModelDescriptorSetBinding);
    
    std::shared_ptr<material> Material = context::Get()->Scene->OverrideMaterial ? context::Get()->Scene->OverrideMaterial : context::Get()->Project.Materials[this->MaterialID]; 
    if(Material->Uniforms) CommandBuffer->BindUniformGroup(Material->Uniforms, MaterialDescriptorSetBinding);
    
    std::shared_ptr<indexedGeometryBuffers> Geometry = context::Get()->Project.Geometries[this->GeometryID];

    CommandBuffer->BindVertexBuffer(Geometry->VertexBuffer);
    CommandBuffer->BindIndexBuffer(Geometry->IndexBuffer, Geometry->Start, gfx::indexType::Uint32);
    CommandBuffer->DrawIndexed(Geometry->Start, Geometry->Count, 1);
}

void mesh::Serialize(std::ofstream &FileStream)
{
    std::vector<u8> Result;

    u32 Object3DType = (u32) object3DType::Mesh;
    FileStream.write((char*)&Object3DType, sizeof(u32));

    FileStream.write((char*)&this->ID, sizeof(u32));
    
    u32 StringLength = this->Name.size();
    FileStream.write((char*)&StringLength, sizeof(u32));
    FileStream.write((char*)(void*)this->Name.data(), StringLength);
    
    FileStream.write((char*)&this->Transform.Matrices, sizeof(transform::matrices));
    FileStream.write((char*)&this->Transform.LocalValues, sizeof(transform::localValues));
    
    FileStream.write((char*)&this->GeometryID, sizeof(u32));
    
    FileStream.write((char*)&this->MaterialID, sizeof(u32));


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
    context *Context = context::Get();
    std::shared_ptr<gfx::context> GfxContext = Context->GfxContext;

    GfxContext->QueueDestroyBuffer(this->UniformBuffer);
    // if(Context->Project.Geometries[this->GeometryID].use_count() == 1)
    // {
    //     Context->Project.Geometries[this->GeometryID]->Destroy();
    // }
}


}