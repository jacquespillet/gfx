#include "Include/Renderer.h"
#include "Include/Context.h"
#include "Include/Scene.h"
#include "Include/Camera.h"
#include "Include/GUI.h"
#include "Include/CameraController.h"

namespace hlgfx
{

void shadowsRenderer::Render(std::shared_ptr<scene> Scene, std::shared_ptr<camera> Camera)
{
    gfx::framebuffer *Framebuffer = gfx::context::Get()->GetFramebuffer(this->RenderTarget);
    std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();    

    CommandBuffer->BeginPass(this->RenderTarget, {0.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0});
    CommandBuffer->SetViewport(0.0f, 0.0f, (float)Framebuffer->Width, (float)Framebuffer->Height, false);
    CommandBuffer->SetScissor(0, 0, Framebuffer->Width, Framebuffer->Height);

    Scene->OverrideMaterial = this->OverrideMaterial;
    Scene->OnRender(Camera);
    Scene->OverrideMaterial = nullptr;

    CommandBuffer->EndPass();
}  


void mainRenderer::Render(std::shared_ptr<scene> Scene, std::shared_ptr<camera> Camera)
{
    gfx::framebuffer *Framebuffer = gfx::context::Get()->GetFramebuffer(this->RenderTarget);
    std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();    

    CommandBuffer->BeginPass(this->RenderTarget, {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});
    CommandBuffer->SetViewport(0.0f, 0.0f, (float)Framebuffer->Width, (float)Framebuffer->Height);
    CommandBuffer->SetScissor(0, 0, Framebuffer->Width, Framebuffer->Height);

    gfx::imgui::Get()->StartFrame();
    context::Get()->CtrlPressed = ImGui::GetIO().KeyCtrl;
    
    context::Get()->GUI->StartFrame();
    context::Get()->GUI->DrawGUI();

    Scene->OnBeforeRender(Camera);
    Scene->OnRender(Camera);
    Scene->OnAfterRender(Camera);
    
    ImGui::GetIO().KeyCtrl = context::Get()->CtrlPressed;
    
    gfx::imgui::Get()->EndFrame(CommandBuffer);
    CommandBuffer->EndPass();

}  

}