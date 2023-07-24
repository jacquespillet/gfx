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
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(Window->GetNativeWindow());
    ImGui_ImplDX12_Init(D12Context->Device.Get(), 2,
        DXGI_FORMAT_R8G8B8A8_UNORM, D12Context->CommonDescriptorHeap.Get(),
        D12Context->GetCPUDescriptorAt(D12Context->CurrentHeapOffset),
        D12Context->GetGPUDescriptorAt(D12Context->CurrentHeapOffset));
    D12Context->CurrentHeapOffset += 2;
    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

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
    io.MouseDown[0] = Clicked;
    // io.mouse
}

void imgui::Cleanup(){
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}


}