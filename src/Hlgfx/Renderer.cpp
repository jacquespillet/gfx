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

    std::vector<gfx::clearColorValues> ClearColour = {{0.0f, 0.0f, 0.0f, 0.0f}};

    CommandBuffer->BeginPass(this->RenderTarget, ClearColour, {1.0f, 0});
    CommandBuffer->SetViewport(0.0f, 0.0f, (float)Framebuffer->Width, (float)Framebuffer->Height, false);
    CommandBuffer->SetScissor(0, 0, Framebuffer->Width, Framebuffer->Height);

    Scene->OverrideMaterial = this->OverrideMaterial;
    Scene->OnRender(Camera);
    Scene->OverrideMaterial = nullptr;

    CommandBuffer->EndPass();
}  


void mainRenderer::Render(std::shared_ptr<scene> Scene, std::shared_ptr<camera> Camera)
{
    RenderTarget = gfx::context::Get()->GetSwapchainFramebuffer();

    gfx::framebuffer *Framebuffer = gfx::context::Get()->GetFramebuffer(this->RenderTarget);
    std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();    

    std::vector<gfx::clearColorValues> ClearColour = {{0.0f, 0.0f, 0.0f, 0.0f}};

    CommandBuffer->BeginPass(this->RenderTarget, ClearColour , {1.0f, 0});
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


// Deferred
deferredRenderer::deferredRenderer()
{
#if 1
    gfx::framebufferCreateInfo FramebufferCreateInfo = {};
    FramebufferCreateInfo.SetSize(context::Get()->Width, context::Get()->Height)
                            .AddColorFormat(gfx::format::R32G32B32A32_SFLOAT) //PositionDepth  
                            .AddColorFormat(gfx::format::R8G8B8A8_UNORM) //Normal
                            .AddColorFormat(gfx::format::R32G32B32A32_UINT) //AlbedoMetallicRoughnessOcclusionOcclusionStrength  
                            .AddColorFormat(gfx::format::R8G8B8A8_UNORM) //Emission  
                            .SetDepthFormat(gfx::format::D16_UNORM)
                            .SetClearColor(0, 0, 0, 0);
    this->RenderTarget = gfx::context::Get()->CreateFramebuffer(FramebufferCreateInfo);

    gfx::framebuffer *Framebuffer = gfx::context::Get()->GetFramebuffer(this->RenderTarget);
    // std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();    

    // this->CompositionPipeline = gfx::context::Get()->CreatePipelineFromFile("resources/Hlgfx/Shaders/Deferred/Composition.json", context::Get()->GfxContext->GetSwapchainFramebuffer());
 
    // this->CompositionMaterial = std::make_shared<customMaterial>("CompositionMaterial", this->CompositionPipeline);    

    // this->CompositionMaterial->Uniforms = std::make_shared<gfx::uniformGroup>();
    // this->CompositionMaterial->Uniforms->Reset()
    //               .AddFramebufferRenderTarget(0, this->GFramebuffer, 0)
    //               .AddFramebufferRenderTarget(0, this->GFramebuffer, 1)
    //               .AddFramebufferRenderTarget(0, this->GFramebuffer, 2)
    //               .AddFramebufferRenderTarget(0, this->GFramebuffer, 3);

    // context::Get()->GfxContext->BindUniformsToPipeline(this->CompositionMaterial->Uniforms, this->CompositionPipeline, 0);
    // this->CompositionMaterial->Uniforms->Update();

    // this->QuadGeometry = GetQuadGeometry();
#endif   
}


void deferredRenderer::Render(std::shared_ptr<scene> Scene, std::shared_ptr<camera> Camera)
{
#if 1
    gfx::framebuffer *Framebuffer = gfx::context::Get()->GetFramebuffer(this->RenderTarget);
    std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();    

    std::vector<gfx::clearColorValues> ClearColour = {{0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}};

    CommandBuffer->BeginPass(this->RenderTarget, ClearColour, {1.0f, 0});
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
#else


    // std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();    
    // // Offscreen
    // {
    //     gfx::framebuffer *Framebuffer = gfx::context::Get()->GetFramebuffer(this->GFramebuffer);

    //     CommandBuffer->BeginPass(this->GFramebuffer, {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0});
    //     CommandBuffer->SetViewport(0.0f, 0.0f, (float)Framebuffer->Width, (float)Framebuffer->Height);
    //     CommandBuffer->SetScissor(0, 0, Framebuffer->Width, Framebuffer->Height);

    //     Scene->OnRender(Camera);
            
    //     CommandBuffer->EndPass();
    // }


    // // Composition
    // {
    //     gfx::framebuffer *Framebuffer = gfx::context::Get()->GetFramebuffer(context::Get()->GfxContext->GetSwapchainFramebuffer());
    //     std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();    

    //     CommandBuffer->BeginPass(context::Get()->GfxContext->GetSwapchainFramebuffer(), {0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0});
    //     CommandBuffer->SetViewport(0.0f, 0.0f, (float)Framebuffer->Width, (float)Framebuffer->Height);
    //     CommandBuffer->SetScissor(0, 0, Framebuffer->Width, Framebuffer->Height);

    //     gfx::imgui::Get()->StartFrame();
    //     context::Get()->CtrlPressed = ImGui::GetIO().KeyCtrl;
        
    //     context::Get()->GUI->StartFrame();
    //     context::Get()->GUI->DrawGUI();

    //     // Draw Quad

    //     CommandBuffer->BindGraphicsPipeline(CompositionPipeline);
    //     CommandBuffer->BindUniformGroup(CompositionMaterial->Uniforms, MaterialDescriptorSetBinding);
    //     CommandBuffer->BindVertexBuffer(this->QuadGeometry->VertexBuffer);
    //     CommandBuffer->BindIndexBuffer(this->QuadGeometry->IndexBuffer, this->QuadGeometry->Start, gfx::indexType::Uint32);
    //     CommandBuffer->DrawIndexed(this->QuadGeometry->Start, this->QuadGeometry->Count, 1);

        
    //     ImGui::GetIO().KeyCtrl = context::Get()->CtrlPressed;
        
    //     gfx::imgui::Get()->EndFrame(CommandBuffer);
    //     CommandBuffer->EndPass();        
    // }
#endif
}  



}