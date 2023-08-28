#include "../Include/Imgui.h"
#include "../Include/Types.h"
#include "../Include/Context.h"
#include "../Include/CommandBuffer.h"
#include "../Include/Framebuffer.h"
#include "../../App/Window.h"

#include "GLCommon.h"
#include "GLContext.h"
#include "GLCommandBuffer.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <ImGuizmo.h>

namespace gfx
{
std::shared_ptr<imgui> imgui::Singleton = {};

imgui *imgui::Get()
{
    return Singleton.get();
}

std::shared_ptr<imgui> imgui::Initialize(std::shared_ptr<context> Context, std::shared_ptr<app::window> Window, framebufferHandle FramebufferHandle)
{
    if(Singleton==nullptr){
        Singleton = std::make_shared<imgui>();
    }
    GET_CONTEXT(GLContext, Context);
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsLight();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(Window->GetHandle(), true);
    ImGui_ImplOpenGL3_Init("#version 450");
  
    return Singleton;
}


void imgui::StartFrame()
{
    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();    
    ImGuizmo::BeginFrame();
}


void imgui::EndFrame(std::shared_ptr<commandBuffer> CommandBuffer)
{
    GET_API_DATA(GLCommandBuffer, glCommandBuffer, CommandBuffer);
    GLCommandBuffer->DrawImgui();
}

void imgui::OnClick(app::mouseButton Button, b8 Clicked){}
void imgui::OnKeyChanged(app::keyCode Button, b8 KeyDown){}

void imgui::Cleanup()
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

}