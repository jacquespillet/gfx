#include "Include/Renderer.h"
#include "Include/Scene.h"
#include "Include/Camera.h"
#include "Include/CameraController.h"

namespace hlgfx
{

void renderer::Render(std::shared_ptr<scene> Scene, std::shared_ptr<camera> Camera)
{
    gfx::framebuffer *Framebuffer = gfx::context::Get()->GetFramebuffer(this->RenderTarget);

    std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();    

    CommandBuffer->BeginPass(this->RenderTarget, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0});
    CommandBuffer->SetViewport(0.0f, 0.0f, (float)Framebuffer->Width, (float)Framebuffer->Height, false);
    CommandBuffer->SetScissor(0, 0, Framebuffer->Width, Framebuffer->Height);

    Scene->OverrideMaterial = this->OverrideMaterial;
    // Scene->OnBeforeRender(Camera);
    Scene->OnRender(Camera);
    Scene->OnAfterRender(Camera);    
    Scene->OverrideMaterial = nullptr;

    CommandBuffer->EndPass();
}  

}