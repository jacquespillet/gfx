#include "Include/Renderer.h"
#include "Include/Context.h"
#include "Include/Scene.h"
#include "Include/Camera.h"
#include "Include/GUI.h"
#include "Include/CameraController.h"
#include "Include/Geometry.h"

namespace hlgfx
{

void renderer::SceneUpdate(){}


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
    
    ImGui::GetIO().KeyCtrl = context::Get()->CtrlPressed;
    
    gfx::imgui::Get()->EndFrame(CommandBuffer);
    CommandBuffer->EndPass();
    

}  


// Deferred
deferredRenderer::deferredRenderer()
{
    context *Context = context::Get();
    std::shared_ptr<gfx::context> GfxContext = Context->GfxContext;

    gfx::framebufferCreateInfo FramebufferCreateInfo = {};
    FramebufferCreateInfo.SetSize(Context->Width, Context->Height)
                            .AddColorFormat(gfx::format::R32G32B32A32_SFLOAT) //PositionDepth  
                            .AddColorFormat(gfx::format::R8G8B8A8_UNORM) //Normal
                            .AddColorFormat(gfx::format::R32G32B32A32_UINT) //AlbedoMetallicRoughnessOcclusionOcclusionStrength  
                            .AddColorFormat(gfx::format::R8G8B8A8_UNORM) //Emission  
                            .SetDepthFormat(gfx::format::D32_SFLOAT)
                            .SetClearColor(0, 0, 0, 0);
    this->RenderTarget = GfxContext->CreateFramebuffer(FramebufferCreateInfo);

    if(context::UseRTX) 
        this->ReflectionImage = GfxContext->CreateImage(Context->Width, Context->Height, gfx::format::R8G8B8A8_UNORM, gfx::imageUsage::bits(gfx::imageUsage::STORAGE | gfx::imageUsage::SHADER_READ | gfx::imageUsage::TRANSFER_SOURCE | gfx::imageUsage::TRANSFER_DESTINATION), gfx::memoryUsage::GpuOnly, nullptr, true);
    else // if rtx isn't enabled, this will just be a black image
        this->ReflectionImage = GfxContext->CreateImage(Context->Width, Context->Height, gfx::format::R8G8B8A8_UNORM, gfx::imageUsage::SHADER_READ, gfx::memoryUsage::GpuOnly, nullptr, false);

    if(context::UseRTX) 
    {
        this->ReflectionsPipeline = GfxContext->CreatePipelineFromFile("resources/Hlgfx/Shaders/RTX/Reflections.json");
        UniformsReflection = std::make_shared<gfx::uniformGroup>();
        UniformsReflection->Reset();  
    }
    
    this->CompositionPipeline = GfxContext->CreatePipelineFromFile("resources/Hlgfx/Shaders/Deferred/Composition.json", Context->GfxContext->GetSwapchainFramebuffer());
    
    this->CompositionMaterial = std::make_shared<customMaterial>("CompositionMaterial", this->CompositionPipeline);    
    this->CompositionMaterial->Uniforms = std::make_shared<gfx::uniformGroup>();
    this->CompositionMaterial->Uniforms->Reset()
                  .AddFramebufferRenderTarget(GBufferPositionBinding, this->RenderTarget, 0)
                  .AddFramebufferRenderTarget(GBufferNormalBinding, this->RenderTarget, 1)
                  .AddFramebufferRenderTarget(GBufferAlbedoBinding, this->RenderTarget, 2)
                  .AddFramebufferRenderTarget(GBufferEmissionBinding, this->RenderTarget, 3)
                  .AddTexture(GBufferReflectionBinding, this->ReflectionImage);

    Context->GfxContext->BindUniformsToPipeline(this->CompositionMaterial->Uniforms, this->CompositionPipeline, GBufferDescriptorSetBinding);
    this->CompositionMaterial->Uniforms->Update();


    this->QuadGeometry = GetQuadGeometry(); 
}

void deferredRenderer::SceneUpdate()
{
    if(context::UseRTX)
    {
        // TODO: Move that to Scene().
        // Build uniforms for ray tracing pipeline
        UniformsReflection->Reset()
            .AddAccelerationStructure(16, context::Get()->Scene->TLAS)
            .AddStorageImage(17, ReflectionImage)
            .AddFramebufferRenderTarget(18, this->RenderTarget, 0)
            .AddFramebufferRenderTarget(19, this->RenderTarget, 1)
            .AddStorageBuffer(20, context::Get()->Scene->VertexBuffer)
            .AddStorageBuffer(21, context::Get()->Scene->OffsetsBuffer)
            .AddStorageBuffer(22, context::Get()->Scene->IndexBuffer)
            .AddStorageBuffer(23, context::Get()->Scene->InstanceMaterialIndices);
            
        context::Get()->GfxContext->BindUniformsToPipeline(this->UniformsReflection, this->ReflectionsPipeline, ReflectionsDescriptorSetBinding);
        context::Get()->GfxContext->BindUniformsToPipeline(context::Get()->CurrentCamera->Uniforms, this->ReflectionsPipeline, CameraDescriptorSetBinding);
        
        this->UniformsReflection->Update();

        //Update bindless descriptor sets
        std::vector<gfx::imageHandle> Images;
        std::vector<std::shared_ptr<texture>> &Textures = context::Get()->Project.Textures;
        for(auto &Texture : Textures)
        {
            Images.push_back(Texture->Handle);
        }
        gfx::context::Get()->UpdateBindlessTextureDescriptorSet(Images);


        //Update bindless descriptor sets
        std::vector<gfx::bufferHandle> MaterialBuffers;
        std::vector<std::shared_ptr<material>> &Materials = context::Get()->Project.Materials;
        for(auto &Material : Materials)
        {   
            MaterialBuffers.push_back(Material->UniformBuffer);
        }
        gfx::context::Get()->UpdateBindlessBufferDescriptorSet(MaterialBuffers);        
    }

}

deferredRenderer::~deferredRenderer()
{  
    this->QuadGeometry->Destroy();
    this->CompositionMaterial = nullptr;
    gfx::context::Get()->DestroyPipeline(this->CompositionPipeline);
    gfx::context::Get()->DestroyImage(this->ReflectionImage);
    gfx::context::Get()->DestroyFramebuffer(this->RenderTarget);
    
    if(context::UseRTX) gfx::context::Get()->DestroyPipeline(this->ReflectionsPipeline);
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
        
        
        CommandBuffer->EndPass();
    }

    if(context::UseRTX) 
    {
        if (UniformsReflection->Bindings.size() != 0)
        {
            CommandBuffer->BindRayTracingPipeline(ReflectionsPipeline);
            CommandBuffer->BindUniformGroup(Camera->Uniforms, CameraDescriptorSetBinding);
            CommandBuffer->BindUniformGroup(UniformsReflection, ReflectionsDescriptorSetBinding);

            CommandBuffer->RayTrace(context::Get()->Width, context::Get()->Height, 1, 0, 1, 2); 
        
            CommandBuffer->TransferLayout(ReflectionImage, gfx::imageUsage::STORAGE, gfx::imageUsage::TRANSFER_DESTINATION);     
            gfx::context::Get()->GetImage(ReflectionImage)->GenerateMipmaps(CommandBuffer);
        }
        else
        {
            CommandBuffer->TransferLayout(ReflectionImage, gfx::imageUsage::STORAGE, gfx::imageUsage::SHADER_READ);     
        }
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
        
        Scene->OnRenderGUI(Camera);
                

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
    
    if(context::UseRTX) CommandBuffer->TransferLayout(ReflectionImage, gfx::imageUsage::SHADER_READ, gfx::imageUsage::STORAGE);       

}  



}