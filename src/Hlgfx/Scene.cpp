#include "gfx/Include/Context.h"

#include "Include/Scene.h"
#include "Include/Mesh.h"
#include "Include/Material.h"

namespace hlgfx
{
scene::scene()
{
    
}
void scene::OnRender(std::shared_ptr<camera> Camera)
{
    gfx::context::Get()->GetCurrentFrameCommandBuffer()->BindUniformGroup(Camera->Uniforms, CameraUniformsBinding);
    object3D::OnRender(Camera);
}

}