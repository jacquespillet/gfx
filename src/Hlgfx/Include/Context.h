#pragma once
#include "App/Window.h"
#include "Gfx/Include/Context.h"
#include "Gfx/Include/Imgui.h"
#include "Types.h"
#include "Camera.h"

#include <memory>
#include <unordered_map>

namespace hlgfx
{

struct scene;
void OnResizeWindow(app::window &Window, app::v2i NewSize);
void OnClickedWindow(app::window &Window, app::mouseButton Button, bool Clicked);
void OnMousePositionChangedWindow(app::window &Window, f64 PosX, f64 PosY);
void OnMouseWheelChangedWindow(app::window &Window, f64 OffsetX, f64 OffsetY);

struct context
{
    static std::shared_ptr<context> Singleton;

    static context *Get();
    static std::shared_ptr<context> Initialize(u32 Width, u32 Height);

    void StartFrame();
    void EndFrame();
    void Update(std::shared_ptr<camera> Camera);
    void OnResize(u32 Width, u32 Height);
    void OnMouseClicked(app::mouseButton Button, bool Clicked);
    void OnMousePositionChanged(f64 NewPosX, f64 NewPosY);
    void OnMouseWheelChanged(f64 OffsetX, f64 OffsetY);
    void Cleanup();
    b8 ShouldClose();

    std::shared_ptr<scene> Scene;

    u32 Width, Height;
    std::shared_ptr<app::window> Window;
	std::shared_ptr<gfx::context> GfxContext;
	gfx::renderPassHandle SwapchainPass;
	gfx::pipelineHandle PipelineHandleSwapchain;
	std::shared_ptr<gfx::swapchain> Swapchain;    

    //That's not optimal : 
    //We create these global pipelines only so we can get their descriptor layouts
    //So we can call BindUniformsToPipeline() on them
    //Then each material instantiates another pipeline
    static const u32 UnlitPipeline = 0;
    std::unordered_map<u32, gfx::pipelineHandle> Pipelines;

    std::shared_ptr<gfx::imgui> Imgui;
	

    //Inputs
    b8 MouseClicked=false;
    b8 MouseReleased =false;
    app::mouseButton ButtonClicked;
    v2i MouseDelta;
    v2i MousePosition;
    b8 MouseMoved=false;
    b8 LeftButtonPressed=false;
    b8 RightButtonPressed=false;
    b8 MouseWheelChanged=false;
    f64 MouseWheelX = 0;
    f64 MouseWheelY = 0;
};

}