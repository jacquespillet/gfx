#include "Include/Mesh.h"
#include "Include/Material.h"
#include "Include/Context.h"
#include "Include/Util.h"
#include "Include/Bindings.h"
#include "gfx/Include/Context.h"
#include <glm/ext.hpp>

namespace hlgfx
{

mesh::mesh() : object3D("Mesh")
{
    this->Material = nullptr;
    this->GeometryBuffers = nullptr;

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

void mesh::DrawMaterial()
{
    this->Material->DrawGUI();
}

std::vector<u8> mesh::Serialize()
{
    std::vector<u8> Result;

    u32 Object3DType = (u32) object3DType::Mesh;
    AddItem(Result, &Object3DType, sizeof(u32));
    
    u32 StringLength = this->Name.size();
    AddItem(Result, &StringLength, sizeof(u32));
    AddItem(Result, (void*)this->Name.data(), StringLength);
    
    sz VertexDataSize = this->GeometryBuffers->VertexData.size() * sizeof(vertex);
    AddItem(Result, &VertexDataSize, sizeof(sz));
    AddItem(Result, this->GeometryBuffers->VertexData.data(), VertexDataSize);
    
    sz IndexDataSize = this->GeometryBuffers->IndexData.size() * sizeof(u32);
    AddItem(Result, &IndexDataSize, sizeof(sz));
    AddItem(Result, this->GeometryBuffers->IndexData.data(), IndexDataSize);

    std::vector<u8> MaterialData = this->Material->Serialize();
    AddItem(Result, MaterialData.data(), MaterialData.size());

    
    AddItem(Result, &this->Transform.Matrices, sizeof(transform::matrices));
    AddItem(Result, &this->Transform.LocalValues, sizeof(transform::localValues));

    u32 NumChildren = this->Children.size();
    AddItem(Result, &NumChildren, sizeof(u32));

    for (sz i = 0; i < NumChildren; i++)
    {
        std::vector<u8> ChildBlob = this->Children[i]->Serialize();
        u32 ChildrenSize = ChildBlob.size();
        AddItem(Result, &ChildrenSize, sizeof(u32));
        AddItem(Result, ChildBlob.data(), ChildBlob.size());
    }


    return Result;
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