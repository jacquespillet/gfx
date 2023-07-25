#include "Include/Mesh.h"
#include "Include/Material.h"
#include "Include/Context.h"
#include "gfx/Include/Context.h"

namespace hlgfx
{

mesh::mesh() : object3D("Mesh")
{
    this->Material = nullptr;
    this->GeometryBuffers.IndexBuffer = gfx::InvalidHandle;
    this->GeometryBuffers.VertexBuffer = gfx::InvalidHandle;

    this->UniformBuffer = gfx::context::Get()->CreateBuffer(sizeof(uniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
    this->Uniforms = std::make_shared<gfx::uniformGroup>();
    this->Uniforms->Reset().AddUniformBuffer(1, this->UniformBuffer);

    //Bind the uniform group to the context pipelines
    context *HighLevelContext = context::Get();
    gfx::context *LowLevelContext = gfx::context::Get();
    for(auto &Pipeline : HighLevelContext->Pipelines)
    {
        LowLevelContext->BindUniformsToPipeline(this->Uniforms, Pipeline.second, ModelUniformsBinding);
    }
    
    this->UniformData.ModelMatrix = this->Transform.LocalToWorld;
    gfx::context::Get()->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(uniformData), 0);

    this->Uniforms->Update();    
}


void mesh::OnRender(std::shared_ptr<camera> Camera) 
{
    if(this->Transform.HasChanged)
    {
        this->UniformData.ModelMatrix = this->Transform.LocalToWorld;
        gfx::context::Get()->CopyDataToBuffer(this->UniformBuffer, &this->UniformData, sizeof(uniformData), 0);
    }
    std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();    
    CommandBuffer->BindUniformGroup(this->Uniforms, ModelUniformsBinding);

    CommandBuffer->BindVertexBuffer(this->GeometryBuffers.VertexBuffer);
    CommandBuffer->BindIndexBuffer(this->GeometryBuffers.IndexBuffer, this->GeometryBuffers.Start, gfx::indexType::Uint32);
    CommandBuffer->DrawIndexed(this->GeometryBuffers.Start, this->GeometryBuffers.Count, 1);
}


}