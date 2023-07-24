#include "Include/Mesh.h"
#include "Include/Material.h"
namespace hlgfx
{

mesh::mesh()
{
    this->Material = nullptr;
    this->GeometryBuffers.IndexBuffer = gfx::InvalidHandle;
    this->GeometryBuffers.VertexBuffer = gfx::InvalidHandle;
}

void mesh::OnRender() 
{
    std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();
    CommandBuffer->BindGraphicsPipeline(this->Material->PipelineHandle);
    CommandBuffer->BindIndexBuffer(this->GeometryBuffers.IndexBuffer, this->GeometryBuffers.Start, gfx::indexType::Uint32);
    CommandBuffer->BindVertexBuffer(this->GeometryBuffers.VertexBuffer);
    CommandBuffer->DrawIndexed(this->GeometryBuffers.Start, this->GeometryBuffers.Count, 1);
}

}