#include "Include/Context.h"
#include "Include/Scene.h"
#include <iostream>
#include "Gfx/Include/CommandBuffer.h"

namespace hlgfx
{
void WindowErrorCallback(const std::string &errorMessage)
{
    std::cout << "Window Error : " << errorMessage << std::endl;
}

void ErrorCallback(const std::string &message)
{
    std::cout << " Error : " << message << std::endl;
}

void InfoCallback(const std::string &message)
{
    std::cout << " Info : " << message << std::endl;
}

void OnResizeWindow(app::window &Window, app::v2i NewSize)
{
    context::Get()->OnResize(NewSize.x, NewSize.y);
}


std::shared_ptr<context> context::Singleton = {};

b8 context::ShouldClose()
{
    return this->Window->ShouldClose();
}

context *context::Get()
{
    return Singleton.get();
}

std::shared_ptr<context> context::Initialize(u32 Width, u32 Height)
{
    if(Singleton==nullptr){
        Singleton = std::make_shared<context>();
    }

    Singleton->Width = Width;
    Singleton->Height = Height;


    gfx::memory::Get()->Init();
	app::windowCreateOptions WindowCreateOptions;
    WindowCreateOptions.Position = app::v2f(300, 100);
    WindowCreateOptions.Size = app::v2f(Width, Height);
    WindowCreateOptions.ErrorCallback = WindowErrorCallback;
#if GFX_API == GFX_VK
    WindowCreateOptions.VersionMajor = 1;
    WindowCreateOptions.VersionMinor = 0;
#elif GFX_API == GFX_GL
    WindowCreateOptions.VersionMajor = 4;
    WindowCreateOptions.VersionMinor = 5;
#endif
    Singleton->Window = std::make_shared<app::window>(WindowCreateOptions);
    Singleton->Window->OnResize = OnResizeWindow;

	// Initialize the graphics API
    gfx::context::initializeInfo ContextInitialize;
    ContextInitialize.Extensions = Singleton->Window->GetRequiredExtensions();
    ContextInitialize.ErrorCallback = ErrorCallback;
    ContextInitialize.InfoCallback = InfoCallback;
    ContextInitialize.Debug = true;
    Singleton->GfxContext = gfx::context::Initialize(ContextInitialize, *Singleton->Window);

    Singleton->Swapchain = Singleton->GfxContext->CreateSwapchain(Width, Height);
    Singleton->SwapchainPass = Singleton->GfxContext->GetDefaultRenderPass();

    Singleton->Scene = std::make_shared<scene>();

    return Singleton;
}

void context::StartFrame()
{
    Window->PollEvents();
    GfxContext->StartFrame();	

    std::shared_ptr<gfx::commandBuffer> CommandBuffer = GfxContext->GetCurrentFrameCommandBuffer();
    CommandBuffer->Begin();    

    CommandBuffer->BeginPass(GfxContext->GetSwapchainFramebuffer(), {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});
    CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
    CommandBuffer->SetScissor(0, 0, Width, Height);
}

void context::Update()
{
    this->Scene->OnUpdate();
    this->Scene->OnBeforeRender();
    this->Scene->OnRender();
    this->Scene->OnAfterRender();
}

void context::EndFrame()
{
    std::shared_ptr<gfx::commandBuffer> CommandBuffer = GfxContext->GetCurrentFrameCommandBuffer();    
    CommandBuffer->EndPass();
    GfxContext->EndFrame();
    GfxContext->Present();    
}

void context::OnResize(u32 Width, u32 Height)
{
    GfxContext->OnResize(Width, Height);
}

void context::Cleanup()
{
    GfxContext->WaitIdle();

    // DestroyProgramSpecific();

    GfxContext->DestroySwapchain();
    GfxContext->Cleanup();

    gfx::memory *Memory = gfx::memory::Get();
    Memory->Destroy();
    delete Memory;    
}

}