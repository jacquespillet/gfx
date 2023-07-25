#include "gfx/Include/Context.h"

#include "Include/Scene.h"
#include "Include/Mesh.h"
#include "Include/Material.h"

namespace hlgfx
{
scene::scene() : object3D("Scene")
{
    
}

void scene::AddObject(std::shared_ptr<object3D> Object)
{
    object3D::AddObject(Object);

    //If it's a mesh, store it in the meshes map as well
    std::shared_ptr<mesh> Mesh = std::dynamic_pointer_cast<mesh>(Object);
    if(Mesh)
    {
        this->Meshes[Mesh->Material->PipelineHandle].push_back(Mesh);
    }
}

void scene::OnRender(std::shared_ptr<camera> Camera)
{
    for(auto &PipelineMeshes : this->Meshes)
    {
        std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();
        CommandBuffer->BindGraphicsPipeline(PipelineMeshes.first);
        CommandBuffer->BindUniformGroup(Camera->Uniforms, CameraUniformsBinding);
        for(sz i=0; i<PipelineMeshes.second.size(); i++)
        {
            PipelineMeshes.second[i]->OnRender(Camera);
        }
    }
}

}