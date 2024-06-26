#include "../Include/ImguiHelper.h"
#include "../Include/Types.h"
#include "../Include/Context.h"
#include "../Include/CommandBuffer.h"
#include "../Include/Framebuffer.h"
#include "../../App/Window.h"

#include "VKCommon.h"
#include "VKContext.h"
#include "VKCommandBuffer.h"
#include "VKFramebuffer.h"
#include "VKRenderPass.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#include <ImGuizmo.h>

namespace gfx
{
std::shared_ptr<imgui> imgui::Singleton = nullptr;

imgui *imgui::Get()
{
    return Singleton.get();
}

bool imgui::IsInitialized()
{
    return Singleton != nullptr;
}

std::shared_ptr<imgui> imgui::Initialize(std::shared_ptr<context> Context, std::shared_ptr<app::window> Window, framebufferHandle FramebufferHandle)
{
    if(Singleton==nullptr){
        Singleton = std::make_shared<imgui>();
    }
    GET_CONTEXT(VKContext, Context);

    framebuffer *Framebuffer = Context->GetFramebuffer(FramebufferHandle);
    renderPass *RenderPass = Context->GetRenderPass(Framebuffer->RenderPass);
    GET_API_DATA(VKRenderPass, vkRenderPassData, RenderPass);


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsLight();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(Window->GetHandle(), true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = VKContext->Instance;
    init_info.PhysicalDevice = VKContext->PhysicalDevice;
    init_info.Device = VKContext->Device;
    init_info.QueueFamily = VKContext->QueueFamilyIndex;
    init_info.Queue = VKContext->DeviceQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = VKContext->DescriptorPool;
    init_info.Allocator = VK_NULL_HANDLE;
    init_info.MinImageCount = VKContext->PresentImageCount;
    init_info.ImageCount = VKContext->PresentImageCount;
    init_info.CheckVkResultFn = VK_NULL_HANDLE;
    init_info.RenderPass = VKRenderPass->NativeHandle;
    ImGui_ImplVulkan_Init(&init_info);
    
    ImGui_ImplVulkan_CreateFontsTexture();
    

    return Singleton;
}


void imgui::StartFrame()
{
    // Start the Dear ImGui frame
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
}


void imgui::EndFrame(std::shared_ptr<commandBuffer> CommandBuffer)
{
    GET_API_DATA(VKCommandBuffer, vkCommandBufferData, CommandBuffer);

    // Rendering
    ImGui::Render();    // Record Imgui Draw Data and draw funcs into command buffer
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), VKCommandBuffer->Handle);

}

void imgui::OnClick(app::mouseButton Button, b8 Clicked){}
void imgui::OnKeyChanged(app::keyCode Button, b8 KeyDown){}

void imgui::Cleanup(){
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

}