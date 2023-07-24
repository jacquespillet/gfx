#pragma once
#include "App/Window.h"
#include "Gfx/Include/Context.h"
#include "Types.h"

#include <memory>
namespace hlgfx
{

struct scene;
void OnResizeWindow(app::window &Window, app::v2i NewSize);

struct context
{
    static std::shared_ptr<context> Singleton;

    static context *Get();
    static std::shared_ptr<context> Initialize(u32 Width, u32 Height);

    void StartFrame();
    void EndFrame();
    void Update();
    void OnResize(u32 Width, u32 Height);
    void Cleanup();
    b8 ShouldClose();

    std::shared_ptr<scene> Scene;

    u32 Width, Height;
    std::shared_ptr<app::window> Window;
	std::shared_ptr<gfx::context> GfxContext;
	gfx::renderPassHandle SwapchainPass;
	gfx::pipelineHandle PipelineHandleSwapchain;
	std::shared_ptr<gfx::swapchain> Swapchain;    
};

}