#include "Include/Renderer.h"
#include "Include/Context.h"
#include "Include/Scene.h"
#include "Include/Camera.h"
#include "Include/GUI.h"
#include "Include/CameraController.h"
#include "Include/Geometry.h"

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
    gfx::framebufferCreateInfo FramebufferCreateInfo = {};
    FramebufferCreateInfo.SetSize(context::Get()->Width, context::Get()->Height)
                            .AddColorFormat(gfx::format::R32G32B32A32_SFLOAT) //PositionDepth  
                            .AddColorFormat(gfx::format::R8G8B8A8_UNORM) //Normal
                            .AddColorFormat(gfx::format::R32G32B32A32_UINT) //AlbedoMetallicRoughnessOcclusionOcclusionStrength  
                            .AddColorFormat(gfx::format::R8G8B8A8_UNORM) //Emission  
                            .SetDepthFormat(gfx::format::D32_SFLOAT)
                            .SetClearColor(0, 0, 0, 0);
    this->RenderTarget = gfx::context::Get()->CreateFramebuffer(FramebufferCreateInfo);

    this->CompositionPipeline = gfx::context::Get()->CreatePipelineFromFile("resources/Hlgfx/Shaders/Deferred/Composition.json", context::Get()->GfxContext->GetSwapchainFramebuffer());
    
    this->CompositionMaterial = std::make_shared<customMaterial>("CompositionMaterial", this->CompositionPipeline);    
    this->CompositionMaterial->Uniforms = std::make_shared<gfx::uniformGroup>();
    this->CompositionMaterial->Uniforms->Reset()
                  .AddFramebufferRenderTarget(GBufferPositionBinding, this->RenderTarget, 0)
                  .AddFramebufferRenderTarget(GBufferNormalBinding, this->RenderTarget, 1)
                  .AddFramebufferRenderTarget(GBufferAlbedoBinding, this->RenderTarget, 2)
                  .AddFramebufferRenderTarget(GBufferEmissionBinding, this->RenderTarget, 3);

    context::Get()->GfxContext->BindUniformsToPipeline(this->CompositionMaterial->Uniforms, this->CompositionPipeline, GBufferDescriptorSetBinding);
    this->CompositionMaterial->Uniforms->Update();

    this->QuadGeometry = GetQuadGeometry(); 
}


void deferredRenderer::Render(std::shared_ptr<scene> Scene, std::shared_ptr<camera> Camera)
{
    std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();    
    {
        gfx::framebuffer *Framebuffer = gfx::context::Get()->GetFramebuffer(this->RenderTarget);

        std::vector<gfx::clearColorValues> ClearColour = {{0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f, 0.0f}};

        CommandBuffer->BeginPass(this->RenderTarget, ClearColour, {1.0f, 0});
        CommandBuffer->SetViewport(0.0f, 0.0f, (float)Framebuffer->Width, (float)Framebuffer->Height);
        CommandBuffer->SetScissor(0, 0, Framebuffer->Width, Framebuffer->Height);

        Scene->OnBeforeRender(Camera);
        Scene->OnRender(Camera);
        Scene->OnAfterRender(Camera);
        
        CommandBuffer->EndPass();
    }


    {
        gfx::framebuffer *Framebuffer = gfx::context::Get()->GetFramebuffer(gfx::context::Get()->GetSwapchainFramebuffer());
        std::vector<gfx::clearColorValues> ClearColour = {{0.0f, 0.0f, 0.0f, 0.0f}};
        CommandBuffer->BeginPass(gfx::context::Get()->GetSwapchainFramebuffer(), ClearColour, {1.0f, 0});
        CommandBuffer->SetViewport(0.0f, 0.0f, (float)Framebuffer->Width, (float)Framebuffer->Height);
        CommandBuffer->SetScissor(0, 0, Framebuffer->Width, Framebuffer->Height);

        gfx::imgui::Get()->StartFrame();
        context::Get()->CtrlPressed = ImGui::GetIO().KeyCtrl;
        
        context::Get()->GUI->StartFrame();
        context::Get()->GUI->DrawGUI();

        CommandBuffer->BindGraphicsPipeline(CompositionPipeline);
        CommandBuffer->BindUniformGroup(Scene->Uniforms, SceneDescriptorSetBinding);
        CommandBuffer->BindUniformGroup(Camera->Uniforms, CameraDescriptorSetBinding);
        CommandBuffer->BindUniformGroup(CompositionMaterial->Uniforms, GBufferDescriptorSetBinding);

        CommandBuffer->BindVertexBuffer(this->QuadGeometry->VertexBuffer);
        CommandBuffer->BindIndexBuffer(this->QuadGeometry->IndexBuffer, this->QuadGeometry->Start, gfx::indexType::Uint32);
        CommandBuffer->DrawIndexed(this->QuadGeometry->Start, this->QuadGeometry->Count, 1);        

        ImGui::GetIO().KeyCtrl = context::Get()->CtrlPressed;
        
        gfx::imgui::Get()->EndFrame(CommandBuffer);
        CommandBuffer->EndPass();    
    }
}  



}