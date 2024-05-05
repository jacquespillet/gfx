#pragma once
#include <vector>
#include <functional>

#include "VertexInput.h"
#include "Types.h"
#include "Pipeline.h"
#include "ResourceManager.h"
#include "Uniform.h"
#include "Image.h"
#include "Buffer.h"
#include <memory>

#include "../Include/Context.h"
#include "../../App/Window.h"

namespace gfx
{

struct imgui
{
    static std::shared_ptr<imgui> Singleton;
    app::window *Window;
    static imgui *Get();
    static std::shared_ptr<imgui> Initialize(std::shared_ptr<context> Context, std::shared_ptr<app::window> Window, framebufferHandle Framebuffer);
    static bool IsInitialized();

    void StartFrame();
    void EndFrame(std::shared_ptr<commandBuffer> CommandBuffer);
    void OnClick(app::mouseButton Button, b8 Clicked);
    void OnKeyChanged(app::keyCode Button, b8 KeyDown);

    void Cleanup();
};
}