#include "../Include/Imgui.h"
#include "../Include/Types.h"
#include "../Include/Context.h"
#include "../Include/CommandBuffer.h"
#include "../../App/Window.h"

#include "D12Common.h"
#include "D12Context.h"
#include "D12CommandBuffer.h"

#include "imgui_impl_win32.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_dx12.h"
#include <d3d12.h>
#include <dxgi1_6.h>

#include <ImGuizmo.h>

#include <iostream>

namespace gfx
{
std::shared_ptr<imgui> imgui::Singleton = {};

imgui *imgui::Get()
{
    return Singleton.get();
}

std::shared_ptr<imgui> imgui::Initialize(std::shared_ptr<context> Context, std::shared_ptr<app::window> Window, framebufferHandle Framebuffer)
{
    if(Singleton==nullptr){
        Singleton = std::make_shared<imgui>();
    }
    GET_CONTEXT(D12Context, Context);

    

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
    ImGui_ImplWin32_Init(Window->GetNativeWindow());
    io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB;
    io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
    io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
    io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
    io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
    io.KeyMap[ImGuiKey_PageUp] = GLFW_KEY_PAGE_UP;
    io.KeyMap[ImGuiKey_PageDown] = GLFW_KEY_PAGE_DOWN;
    io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
    io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
    io.KeyMap[ImGuiKey_Insert] = GLFW_KEY_INSERT;
    io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
    io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
    io.KeyMap[ImGuiKey_Space] = GLFW_KEY_SPACE;
    io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
    io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
    io.KeyMap[ImGuiKey_KeyPadEnter] = GLFW_KEY_KP_ENTER;
    io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
    io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
    io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
    io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
    io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
    io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;    
    ImGui_ImplDX12_Init(D12Context->Device.Get(), 2,
        DXGI_FORMAT_R8G8B8A8_UNORM, D12Context->CommonDescriptorHeap.Get(),
        D12Context->GetCPUDescriptorAt(D12Context->CurrentHeapOffset),
        D12Context->GetGPUDescriptorAt(D12Context->CurrentHeapOffset));
    D12Context->CurrentHeapOffset += 2;


    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    return Singleton;
}


void imgui::StartFrame()
{
     // Start the Dear ImGui frame
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}


void imgui::EndFrame(std::shared_ptr<commandBuffer> CommandBuffer)
{
    GET_API_DATA(D12CommandBuffer, d3d12CommandBufferData, CommandBuffer);

    ImGui::Render();
    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), D12CommandBuffer->CommandList.Get());
}

void imgui::OnClick(app::mouseButton Button, b8 Clicked)
{
    ImGuiIO& io = ImGui::GetIO();
    if(Button == app::mouseButton::LEFT)
        io.MouseDown[0] = Clicked;
    if(Button == app::mouseButton::RIGHT)
        io.MouseDown[1] = Clicked;
    if(Button == app::mouseButton::MIDDLE)
        io.MouseDown[2] = Clicked;
}

void imgui::OnKeyChanged(app::keyCode Button, b8 KeyDown)
{
    ImGuiIO& io = ImGui::GetIO();
    io.KeysDown[(sz)Button] = KeyDown;
    
    io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
    io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
    io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];

    

    if(KeyDown)
        io.AddInputCharacter((int)Button);
}

void imgui::Cleanup(){
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}


}