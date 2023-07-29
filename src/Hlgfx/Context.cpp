#include "Include/Context.h"
#include "Include/Scene.h"
#include "Include/CameraController.h"
#include "Include/Material.h"
#include <iostream>
#include "Gfx/Include/CommandBuffer.h"
#include <nfd.h>

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

void OnClickedWindow(app::window &Window, app::mouseButton Button, bool Clicked)
{
    context::Get()->OnMouseClicked(Button, Clicked);
}

void OnMousePositionChangedWindow(app::window &Window, f64 PosX, f64 PosY)
{
    context::Get()->OnMousePositionChanged(PosX, PosY);
}

void OnMouseWheelChangedWindow(app::window &Window, f64 OffsetX, f64 OffsetY)
{
    context::Get()->OnMouseWheelChanged(OffsetX, OffsetY);
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



    gfx::memory::Get()->Init();
	app::windowCreateOptions WindowCreateOptions = {};
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
    Singleton->Window->OnMouseChanged = OnClickedWindow;
    Singleton->Window->OnMousePositionChanged = OnMousePositionChangedWindow;
    Singleton->Window->OnMouseWheelChanged = OnMouseWheelChangedWindow;

    Singleton->Width = Singleton->Window->Width;
    Singleton->Height = Singleton->Window->Height;

	// Initialize the graphics API
    gfx::context::initializeInfo ContextInitialize;
    ContextInitialize.Extensions = Singleton->Window->GetRequiredExtensions();
    ContextInitialize.ErrorCallback = ErrorCallback;
    ContextInitialize.InfoCallback = InfoCallback;
    ContextInitialize.Debug = true;
    Singleton->GfxContext = gfx::context::Initialize(ContextInitialize, *Singleton->Window);

    Singleton->Swapchain = Singleton->GfxContext->CreateSwapchain(Singleton->Width, Singleton->Height);
    Singleton->SwapchainPass = Singleton->GfxContext->GetDefaultRenderPass();

    Singleton->Scene = std::make_shared<scene>();

    Singleton->Pipelines[UnlitPipeline] = gfx::context::Get()->CreatePipelineFromFile("resources/Hlgfx/Shaders/Unlit/Unlit.json");

    Singleton->Imgui = gfx::imgui::Initialize(Singleton->GfxContext, Singleton->Window, Singleton->SwapchainPass);
    
    std::vector<u8> ColorData = {0,0,0,0};
    gfx::imageData ImageData = {};
    ImageData.ChannelCount = 4;
    ImageData.Data = ColorData.data();
    ImageData.DataSize = ColorData.size();
    ImageData.Width = 1;
    ImageData.Height = 1;
    ImageData.Format = gfx::format::R8G8B8A8_UNORM;
    ImageData.Type = gfx::type::UNSIGNED_BYTE;
    gfx::imageCreateInfo ImageCreateInfo = 
    {
        {0.0f,0.0f,0.0f,0.0f},
        gfx::samplerFilter::Linear,
        gfx::samplerFilter::Linear,
        gfx::samplerWrapMode::ClampToBorder,
        gfx::samplerWrapMode::ClampToBorder,
        gfx::samplerWrapMode::ClampToBorder,
        true
    };    
    defaultTextures::BlackTexture = Singleton->GfxContext->CreateImage(ImageData, ImageCreateInfo);

    ColorData[2] = 255;
    defaultTextures::BlueTexture = Singleton->GfxContext->CreateImage(ImageData, ImageCreateInfo);
    
    ColorData[0] = 255; ColorData[1] = 255; ColorData[2] = 255; ColorData[3] = 255;
    defaultTextures::WhiteTexture = Singleton->GfxContext->CreateImage(ImageData, ImageCreateInfo);

    return Singleton;
}

void context::StartFrame()
{
    this->MouseClicked=false;
    this->MouseReleased=false;
    this->MouseMoved=false;
    this->MouseWheelChanged=false;
    Window->PollEvents();
    
    GfxContext->StartFrame();	

    std::shared_ptr<gfx::commandBuffer> CommandBuffer = GfxContext->GetCurrentFrameCommandBuffer();
    CommandBuffer->Begin();    

    CommandBuffer->BeginPass(GfxContext->GetSwapchainFramebuffer(), {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});
    CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
    CommandBuffer->SetScissor(0, 0, Width, Height);

    Imgui->StartFrame();
    IsInteractingGUI = (ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused() || ImGui::IsAnyWindowHovered() || ImGuizmo::IsUsingAny());

    this->Scene->OnEarlyUpdate();

    this->DrawGUI();
}

void context::Update(std::shared_ptr<camera> Camera)
{

    Camera->Controls->OnUpdate();
    this->Scene->OnUpdate();
    this->Scene->OnBeforeRender(Camera);
    this->Scene->OnRender(Camera);
    this->Scene->OnAfterRender(Camera);
}

void context::EndFrame()
{
    std::shared_ptr<gfx::commandBuffer> CommandBuffer = GfxContext->GetCurrentFrameCommandBuffer();    
    Imgui->EndFrame(CommandBuffer);
    CommandBuffer->EndPass();
    GfxContext->EndFrame();
    GfxContext->Present();   
     
}

void context::OnResize(u32 Width, u32 Height)
{
    GfxContext->OnResize(Width, Height);
}

void context::OnMouseClicked(app::mouseButton Button, bool Clicked)
{
    Imgui->OnClick(Button, Clicked);
    
    if(Clicked) 
    {
        this->MouseClicked=true;
        if(Button == app::mouseButton::LEFT) this->LeftButtonPressed=true;
        if(Button == app::mouseButton::RIGHT) this->RightButtonPressed=true;
    }
    if(!Clicked) 
    {
        this->MouseReleased=true;
        if(Button == app::mouseButton::LEFT) this->LeftButtonPressed=false;
        if(Button == app::mouseButton::RIGHT) this->RightButtonPressed=false;
    }
    this->ButtonClicked = Button;
}

void context::OnMousePositionChanged(f64 NewPosX, f64 NewPosY)
{
    this->MouseMoved=true;
    this->MouseDelta.x = NewPosX - this->MousePosition.x;
    this->MouseDelta.y = NewPosY - this->MousePosition.y;
    this->MousePosition.x = NewPosX;
    this->MousePosition.y = NewPosY;
}

void context::OnMouseWheelChanged(f64 OffsetX, f64 OffsetY)
{
    this->MouseWheelChanged=true;
    this->MouseWheelX = OffsetX;
    this->MouseWheelY = OffsetY;
}

void context::DrawGuizmoGUI()
{
    ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(100, 100), ImGuiCond_Always);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.3, 0.3, 0.3, 0.3));
    ImGui::Begin("Guizmo", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
    if (ImGui::Button("Translate"))
        CurrentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::Button("Rotate"))
        CurrentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::Button("Scale"))
        CurrentGizmoOperation = ImGuizmo::SCALE;
    
    b8 IsLocal = CurrentGizmoMode == ImGuizmo::LOCAL;
    if(ImGui::Checkbox("Local", &IsLocal))
    {
        CurrentGizmoMode = IsLocal ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
    }
    ImGui::End();
    ImGui::PopStyleColor();
}

void context::DrawMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("New"))
            {
                this->Scene->Clear();
            }
            if(ImGui::MenuItem("Save"))
            {
                nfdchar_t *OutPath = NULL;
                nfdresult_t Result = NFD_SaveDialog( NULL, NULL, &OutPath );
                if ( Result == NFD_OKAY ) {
                    this->Scene->SaveToFile(OutPath);
                }
            }
            if(ImGui::MenuItem("Load"))
            {
                nfdchar_t *OutPath = NULL;
                nfdresult_t Result = NFD_OpenDialog( NULL, NULL, &OutPath );
                if ( Result == NFD_OKAY ) {
                    this->Scene->LoadFromFile(OutPath);
                }                
            }
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Edit"))
        {
            if(ImGui::MenuItem("Clone"))
            {
                if(Scene->NodeClicked)
                {

                }
            }
            if(ImGui::MenuItem("Delete"))
            {
                if(Scene->NodeClicked)
                {
                    Scene->DeleteObject(Scene->NodeClicked);
                    Scene->NodeClicked=nullptr;
                }
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Add"))
        {
            if (ImGui::BeginMenu("Add Object")) 
            {
                if(ImGui::MenuItem("Empty"))
                {
                    std::shared_ptr<hlgfx::object3D> Empty = std::make_shared<hlgfx::object3D>("Empty");
                    if(this->Scene->NodeClicked != nullptr)
                    {
                        this->Scene->NodeClicked->AddObject(Empty);
                    }
                    else
                    {
                        this->Scene->AddObject(Empty);
                    }
                }
                if(ImGui::MenuItem("Quad"))
                {
                    std::shared_ptr<hlgfx::mesh> Mesh = std::make_shared<hlgfx::mesh>();
                    Mesh->GeometryBuffers = hlgfx::GetTriangleGeometry();
                    Mesh->Material = std::make_shared<hlgfx::unlitMaterial>();
                    if(this->Scene->NodeClicked != nullptr)
                    {
                        this->Scene->NodeClicked->AddObject(Mesh);
                    }
                    else
                    {
                        this->Scene->AddObject(Mesh);
                    }
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void context::DrawGUI()
{
    DrawGuizmoGUI();
    DrawMainMenuBar();


    this->Scene->DrawGUI();
}

void context::Cleanup()
{
    GfxContext->WaitIdle();

    GfxContext->QueueDestroyImage(defaultTextures::BlackTexture);
    GfxContext->QueueDestroyImage(defaultTextures::WhiteTexture);
    GfxContext->QueueDestroyImage(defaultTextures::BlueTexture);

    //Clears the scene, effectively dereferences all the pointers to call their destructors before we clean up
    this->Scene->Children = std::vector<std::shared_ptr<object3D>>();
    this->Scene->Meshes = std::unordered_map<gfx::pipelineHandle, std::vector<std::shared_ptr<mesh>>>();
    this->Scene->NodeClicked = nullptr;
    
    GfxContext->ProcessDeletionQueue();

    for(auto &Pipeline : this->Pipelines)
    {
        GfxContext->DestroyPipeline(Pipeline.second);
    }

    Imgui->Cleanup();

    GfxContext->DestroySwapchain();
    GfxContext->Cleanup();

    gfx::memory *Memory = gfx::memory::Get();
    Memory->Destroy();
    delete Memory;    
}

context::~context()
{
    Cleanup();
}

}