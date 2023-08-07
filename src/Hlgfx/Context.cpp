#include "Include/Context.h"
#include "Include/Scene.h"
#include "Include/CameraController.h"
#include "Include/Material.h"
#include "Include/Util.h"
#include "Loaders/GLTF.h"
#include "Gfx/Include/CommandBuffer.h"

#include <iostream>
#include <nfd.h>
#include <filesystem>

namespace hlgfx
{
UUIDv4::UUIDGenerator<std::mt19937_64> context::UUIDGenerator = UUIDv4::UUIDGenerator<std::mt19937_64>();

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


    Singleton->Pipelines[UnlitPipeline] = gfx::context::Get()->CreatePipelineFromFile("resources/Hlgfx/Shaders/Unlit/Unlit.json");

    Singleton->Imgui = gfx::imgui::Initialize(Singleton->GfxContext, Singleton->Window, Singleton->SwapchainPass);
    
    struct rgba {uint8_t r, g, b, a;};
    u32 TexWidth = 64;
    u32 TexHeight = 64;
    std::vector<rgba> ColorData(TexWidth * TexHeight);
    
    rgba Color = {0,0,0,255};
    for (sz i = 0; i < TexWidth * TexHeight; i++)
        ColorData[i] = Color;
    
    gfx::imageData ImageData = {};
    ImageData.ChannelCount = 4;
    ImageData.Data = (u8*)ColorData.data();
    ImageData.DataSize = ColorData.size() * sizeof(rgba);
    ImageData.Width = TexWidth;
    ImageData.Height = TexHeight;
    ImageData.Format = gfx::format::R8G8B8A8_UNORM;
    ImageData.Type = gfx::type::UNSIGNED_BYTE;
    gfx::imageCreateInfo ImageCreateInfo = 
    {
        {0.0f,0.0f,0.0f,0.0f},
        gfx::samplerFilter::Linear,
        gfx::samplerFilter::Linear,
        gfx::samplerWrapMode::Repeat,
        gfx::samplerWrapMode::Repeat,
        gfx::samplerWrapMode::Repeat,
        true
    };    

    defaultTextures::BlackTexture->Handle = Singleton->GfxContext->CreateImage(ImageData, ImageCreateInfo);


    Color = {0,0,255,255};
    for (sz i = 0; i < TexWidth * TexHeight; i++)
        ColorData[i] = Color;
    defaultTextures::BlueTexture->Handle = Singleton->GfxContext->CreateImage(ImageData, ImageCreateInfo);
    
    Color = {255,255,255,255};
    for (sz i = 0; i < TexWidth * TexHeight; i++)
        ColorData[i] = Color;
    defaultTextures::WhiteTexture->Handle = Singleton->GfxContext->CreateImage(ImageData, ImageCreateInfo);

    return Singleton;
}

gfx::pipelineCreation context::GetPipelineCreation(materialFlags::bits Flags)
{

    gfx::pipelineCreation PipelineCreation = {};

    std::string ShaderParentPath;
    std::string ShaderFileName;
    if(Flags & materialFlags::Unlit)
    {
        PipelineCreation.Name = gfx::AllocateCString("Unlit");
        ShaderParentPath = "resources/Hlgfx/Shaders/Unlit/";
        if(GFX_API == GFX_VK || GFX_API == GFX_GL)
        {
            ShaderFileName = "Unlit.glsl";
        }
        else
        {
            ShaderFileName = "Unlit.hlsl";
        }
    }


    //
    std::string ApiDefinition;
    ApiDefinition += "#define VK " + std::to_string(GFX_VK) + "\n";
    ApiDefinition += "#define GL " + std::to_string(GFX_GL) + "\n";
    ApiDefinition += "#define D3D11 " + std::to_string(GFX_D3D11) + "\n";
    ApiDefinition += "#define GFX_D3D12 " + std::to_string(GFX_D3D12) + "\n";
    if(GFX_API == GFX_VK)
    {
        ApiDefinition += "#define GRAPHICS_API VK\n";
    }
    if(GFX_API == GFX_GL)
    {
        ApiDefinition += "#define GRAPHICS_API GL\n";
    }
    if(GFX_API == GFX_D3D12)
    {
        ApiDefinition += "#define GRAPHICS_API D3D12\n";
    }
    if(GFX_API == GFX_D3D11)
    {
        ApiDefinition += "#define GRAPHICS_API D3D11\n";
    }

    std::string VertexCode;
    gfx::ShaderConcatenate(ShaderFileName, VertexCode, ShaderParentPath);
    std::string VertexCustomDefines = "#define VERTEX\n";
    VertexCustomDefines += ApiDefinition;
    VertexCode = std::regex_replace(VertexCode, std::regex("CUSTOM_DEFINES"), VertexCustomDefines);
    const char *VertexCodeCStr = gfx::AllocateCString(VertexCode.c_str());
    const char *VertexFileNameCStr = gfx::AllocateCString(ShaderParentPath + "/" + ShaderFileName);
    PipelineCreation.Shaders.AddStage(VertexCodeCStr, VertexFileNameCStr, (u32)strlen(VertexCodeCStr), gfx::shaderStageFlags::bits::Vertex);

    std::string FragmentCode;
    gfx::ShaderConcatenate(ShaderFileName, FragmentCode, ShaderParentPath);
    std::string FragmentCustomDefines = "#define FRAGMENT\n";
    FragmentCustomDefines += ApiDefinition;
    FragmentCode = std::regex_replace(FragmentCode, std::regex("CUSTOM_DEFINES"), FragmentCustomDefines);
    const char *FragmentCodeCStr = gfx::AllocateCString(FragmentCode.c_str());
    const char *FragmentFileNameCStr = gfx::AllocateCString(ShaderParentPath + "/" + ShaderFileName);
    PipelineCreation.Shaders.AddStage(FragmentCodeCStr, FragmentFileNameCStr, (u32)strlen(FragmentCodeCStr), gfx::shaderStageFlags::bits::Fragment);

    
    PipelineCreation.VertexInput.NumVertexStreams=0;
    gfx::vertexStream VertexStream{};
    VertexStream.Binding = 0;
    VertexStream.Stride = sizeof(vertex);
    VertexStream.InputRate = gfx::vertexInputRate::PerVertex;
    PipelineCreation.VertexInput.AddVertexStream(VertexStream);        

    PipelineCreation.VertexInput.NumVertexAttributes=0;
    gfx::vertexAttribute VertexAttribute0{};
    VertexAttribute0.Location = 0;
    VertexAttribute0.Binding = 0;
    VertexAttribute0.Offset = 0;
    VertexAttribute0.Format = gfx::vertexComponentFormat::Float4;
    VertexAttribute0.SemanticIndex = 0;
    PipelineCreation.VertexInput.AddVertexAttribute(VertexAttribute0);
    gfx::vertexAttribute VertexAttribute1{};
    VertexAttribute1.Location = 1;
    VertexAttribute1.Binding = 0;
    VertexAttribute1.Offset = sizeof(v4f);
    VertexAttribute1.Format = gfx::vertexComponentFormat::Float4;
    VertexAttribute1.SemanticIndex = 1;
    PipelineCreation.VertexInput.AddVertexAttribute(VertexAttribute1);

    u8 T = Flags & materialFlags::DepthTestEnabled;

    PipelineCreation.DepthStencil.DepthEnable = (u8)((b8)(Flags & materialFlags::DepthTestEnabled));
    PipelineCreation.DepthStencil.DepthWriteEnable = (u8)((b8)(Flags & materialFlags::DepthWriteEnabled));
    PipelineCreation.DepthStencil.DepthComparison = gfx::compareOperation::LessOrEqual;
    

    gfx::blendState &BlendState = PipelineCreation.BlendState.AddBlendState();
    BlendState.BlendEnabled = Flags & materialFlags::BlendEnabled;
    BlendState.SeparateBlend=false;
    if(BlendState.BlendEnabled)
        BlendState.SetColor(gfx::blendFactor::SrcAlpha, gfx::blendFactor::OneMinusSrcAlpha, gfx::blendOperation::Add);

    PipelineCreation.Rasterization.CullMode = (Flags & materialFlags::CullModeOn) ? gfx::cullMode::Back : gfx::cullMode::None;

    PipelineCreation.RenderPassHandle = gfx::context::Get()->SwapchainRenderPass;

    return PipelineCreation;    
}

gfx::pipelineHandle context::GetPipeline(materialFlags::bits Flags)
{
    if(this->AllPipelines.find(Flags) != this->AllPipelines.end())
    {
        return this->AllPipelines[Flags];    
    }
    return gfx::InvalidHandle;
}

gfx::pipelineHandle context::CreateOrGetPipeline(materialFlags::bits Flags)
{
    if(this->AllPipelines.find(Flags) == this->AllPipelines.end())
    {
        gfx::pipelineCreation Creation = GetPipelineCreation(Flags);
        this->AllPipelines[Flags] = gfx::context::Get()->CreatePipeline(Creation);
    }

    return this->AllPipelines[Flags];
}

void context::StartFrame()
{
    this->MouseClicked=false;
    this->MouseReleased=false;
    this->MouseMoved=false;
    this->MouseWheelChanged=false;
    Window->PollEvents();
    
    this->Scene->OnEarlyUpdate();
    
    GfxContext->StartFrame();	

    std::shared_ptr<gfx::commandBuffer> CommandBuffer = GfxContext->GetCurrentFrameCommandBuffer();
    CommandBuffer->Begin();    

    CommandBuffer->BeginPass(GfxContext->GetSwapchainFramebuffer(), {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});
    CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
    CommandBuffer->SetScissor(0, 0, Width, Height);

    Imgui->StartFrame();
    IsInteractingGUI = (ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused() || ImGui::IsAnyWindowHovered() || ImGuizmo::IsUsingAny());


    this->DrawGUI();
}

std::string context::GetUUID()
{
    UUIDv4::UUID UUID = UUIDGenerator.getUUID();
    std::string Result = UUID.bytes();
    return Result;
}

void context::AddTextureToProject(std::shared_ptr<texture> Texture)
{
    this->Project.Textures[Texture->UUID] = Texture;
}
void context::AddMaterialToProject(std::shared_ptr<material> Material)
{
    this->Project.Materials[Material->UUID] = Material;
}

void context::AddMeshToProject(std::shared_ptr<mesh> Mesh)
{
    this->Project.Geometries[Mesh->GeometryBuffers->UUID] = Mesh->GeometryBuffers;
    this->Project.Materials[Mesh->Material->UUID] = Mesh->Material;
    for(auto &Texture : Mesh->Material->AllTextures)
    {
        this->Project.Textures[Texture.first] = Texture.second;
    }
}

void context::AddSceneToProject(std::shared_ptr<scene> Scene)
{
    this->Project.Scenes[Scene->UUID] = Scene;
    this->Scene = Scene;
}


void context::AddObjectToProject(std::shared_ptr<object3D> Object, u32 Level)
{
    if (Level == 0)
    {

        //Check unicity of name
        u32 Count = 0;
        std::string BaseName = Object->Name;
        while(true)
        {
            b8 OK = true;
            for (auto &ProjectObject : this->Project.Objects)
            {
                //If another object has the same name, we append the count, and set OK to false so that we iterate again for this new name
                if(ProjectObject.second->Name == Object->Name)
                {
                    Object->Name = BaseName + "_" + std::to_string(Count);
                    OK = false;
                    Count++;
                }
            }

            if(OK) break;
        }
        this->Project.Objects[Object->UUID] = (Object);
    }
    
    
    std::shared_ptr<mesh> Mesh = std::dynamic_pointer_cast<mesh>(Object);
    if(Mesh)
    {
        AddMeshToProject(Mesh);
    }
    for (sz i = 0; i < Object->Children.size(); i++)
    {
        AddObjectToProject(Object->Children[i], 1);
    }
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

void context::DrawObjectMenu()
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
}

void context::AddObjectMenu()
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
        Mesh->Material = std::make_shared<hlgfx::unlitMaterial>("New Material");
        if(this->Scene->NodeClicked != nullptr)
        {
            this->Scene->NodeClicked->AddObject(Mesh);
        }
        else
        {
            this->Scene->AddObject(Mesh);
        }
    }
}
void context::DrawAssetsWindow()
{
    ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Appearing);
    ImGui::Begin("Assets", 0);
    ImGuiTabBarFlags TabBarFlags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("Assets", TabBarFlags))
    {
        if(ImGui::BeginTabItem("Objects"))
        {
            for (auto &Object : this->Project.Objects)
            {
                ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
                if(this->SelectedObject3D.get() == Object.second.get()) Flags |= ImGuiTreeNodeFlags_Selected;
                ImGui::TreeNodeEx(Object.second->Name.c_str(), Flags);
                if(ImGui::IsItemClicked())
                {
                    if(this->SelectedObject3D == Object.second) this->SelectedObject3D = nullptr;
                    else this->SelectedObject3D = Object.second;
                }
                ImGui::TreePop();
            }
            
            if(this->SelectedObject3D)
            {
                ImGui::BeginChild("Actions");
                if(ImGui::Button("Add To Scene"))
                {
                    this->Scene->AddObject(this->SelectedObject3D->Clone());
                }
                if(ImGui::Button("Duplicate"))
                {
                    this->AddObjectToProject(this->SelectedObject3D->Clone());
                }
                ImGui::EndChild();
            }
            
            ImGui::Separator();
            
            if(ImGui::Button("Import"))
            {
                nfdchar_t *OutPath = NULL;
                nfdresult_t Result = NFD_OpenDialog( NULL, NULL, &OutPath );
                if ( Result == NFD_OKAY ) {
                    std::shared_ptr<hlgfx::object3D> Mesh = hlgfx::loaders::gltf::Load(OutPath);
                    this->AddObjectToProject(Mesh);
                }                    
            }

            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Materials"))
        {
            if (ImGui::Button("Add New"))
            {
                this->AddMaterialToProject(std::make_shared<unlitMaterial>("New Material"));
            }

            for (auto &Material : this->Project.Materials)
            {
                ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
                if(this->SelectedMaterial.get() == Material.second.get()) Flags |= ImGuiTreeNodeFlags_Selected;
                ImGui::TreeNodeEx(Material.second->Name.c_str(), Flags);
                if(ImGui::IsItemClicked())
                {
                    if(this->SelectedMaterial == Material.second) this->SelectedMaterial = nullptr;
                    else this->SelectedMaterial = Material.second;
                }

                ImGui::TreePop();
            }

            if(this->SelectedMaterial)
            {
                ImGui::BeginChild("Material");
                if(ImGui::Button("Duplicate"))
                {
                    this->AddMaterialToProject(this->SelectedMaterial->Clone());
                }
                this->SelectedMaterial->DrawGUI();
                ImGui::EndChild();
            }
            ImGui::Separator();


            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Textures"))
        {
            if(ImGui::Button("Import"))
            {
                nfdchar_t *OutPath = NULL;
                nfdresult_t Result = NFD_OpenDialog( NULL, NULL, &OutPath );
                if ( Result == NFD_OKAY ) {
                    gfx::imageData ImageData = gfx::ImageFromFile(OutPath);
                    gfx::imageCreateInfo ImageCreateInfo = 
                    {
                        {0.0f,0.0f,0.0f,0.0f},
                        gfx::samplerFilter::Linear,
                        gfx::samplerFilter::Linear,
                        gfx::samplerWrapMode::Repeat,
                        gfx::samplerWrapMode::Repeat,
                        gfx::samplerWrapMode::Repeat,
                        true
                    };
                    gfx::imageHandle NewImage = gfx::context::Get()->CreateImage(ImageData, ImageCreateInfo);                
                    std::shared_ptr<texture> Texture = std::make_shared<texture>(FileNameFromPath(OutPath), NewImage);
                    this->AddTextureToProject(Texture);
                }                            
            }
            for (auto &Texture : this->Project.Textures)
            {
                ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
                if(this->SelectedTexture.get() == Texture.second.get()) Flags |= ImGuiTreeNodeFlags_Selected;
                ImGui::TreeNodeEx(Texture.second->Name.c_str(), Flags);
                if(ImGui::IsItemClicked())
                {
                    if(this->SelectedTexture == Texture.second) this->SelectedTexture =nullptr;
                    else this->SelectedTexture = Texture.second;
                }
                ImGui::TreePop();
            }
            
            if(this->SelectedTexture!=nullptr)
            {
                ImGui::BeginChild("Texture");
            
                ImGui::EndChild();
            }

            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Geometries"))
        {
            for (auto &Geometries : this->Project.Geometries)
            {
                ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
                if(this->SelectedIndexedGeometryBuffers.get() == Geometries.second.get()) Flags |= ImGuiTreeNodeFlags_Selected;
                ImGui::TreeNodeEx(Geometries.second->Name.c_str(), Flags);
                if(ImGui::IsItemClicked())
                {
                    this->SelectedIndexedGeometryBuffers = Geometries.second;
                }
                ImGui::TreePop();
            }
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Scenes"))
        {
            for (auto &Scene : this->Project.Scenes)
            {
                ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
                if(this->SelectedScene.get() == Scene.second.get()) Flags |= ImGuiTreeNodeFlags_Selected;
                ImGui::TreeNodeEx(Scene.second->Name.c_str(), Flags);
                if(ImGui::IsItemClicked())
                {
                    this->SelectedScene = Scene.second;
                }
                ImGui::TreePop();
            }

            if(this->SelectedScene != nullptr)
            {
                if(ImGui::Button("Open"))
                {
                    this->Scene = this->SelectedScene;
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void context::DrawMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("New Scene"))
            {
                std::shared_ptr<scene> NewScene = std::make_shared<scene>("New_Scene_" + this->Project.Scenes.size());
                this->Project.Scenes[NewScene->UUID] = NewScene;
                this->Scene = NewScene;
            }
            if(ImGui::MenuItem("Import GLTF"))
            {
                nfdchar_t *OutPath = NULL;
                nfdresult_t Result = NFD_OpenDialog( NULL, NULL, &OutPath );
                if ( Result == NFD_OKAY ) {
                    std::shared_ptr<hlgfx::object3D> Mesh = hlgfx::loaders::gltf::Load(OutPath);
                    this->AddObjectToProject(Mesh);
                }                
            }
            ImGui::Separator();
            if(ImGui::MenuItem("Save Project"))
            {
                nfdchar_t *OutPath = NULL;
                nfdresult_t Result = NFD_PickFolder(NULL, &OutPath );
                if ( Result == NFD_OKAY ) {
                    this->SaveProjectToFolder(OutPath);
                }
            }
            if(ImGui::MenuItem("Load Project"))
            {
                nfdchar_t *OutPath = NULL;
                nfdresult_t Result = NFD_PickFolder(NULL, &OutPath );
                if ( Result == NFD_OKAY ) {
                    this->LoadProjectFromFolder(OutPath);
                }                
            }
            ImGui::Separator();
            ImGui::EndMenu();
        }
        if(ImGui::BeginMenu("Edit"))
        {
            DrawObjectMenu();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Add"))
        {
            if (ImGui::BeginMenu("Add Object")) 
            {
                AddObjectMenu();
                ImGui::EndMenu();
            }
            ImGui::EndMenu();
        }
        if(ImGui::MenuItem("Assets"))
        {
            this->ShowAssetsWindow = !this->ShowAssetsWindow;
        }
        ImGui::EndMainMenuBar();
    }
}

void context::DrawGUI()
{
    DrawGuizmoGUI();
    DrawMainMenuBar();
    if(ShowAssetsWindow) DrawAssetsWindow();


    this->Scene->DrawGUI();
}

void context::Cleanup()
{
    GfxContext->WaitIdle();

    this->Project.Geometries.clear();
    this->Project.Materials.clear();
    this->Project.Objects.clear();
    this->Project.Textures.clear();

    //Clears the scene, effectively dereferences all the pointers to call their destructors before we clean up
    this->Scene->Children = std::vector<std::shared_ptr<object3D>>();
    this->Scene->Meshes = std::unordered_map<gfx::pipelineHandle, std::vector<std::shared_ptr<mesh>>>();
    this->Scene->NodeClicked = nullptr;
    
    GfxContext->QueueDestroyImage(defaultTextures::BlackTexture->Handle);
    GfxContext->QueueDestroyImage(defaultTextures::WhiteTexture->Handle);
    GfxContext->QueueDestroyImage(defaultTextures::BlueTexture->Handle);
    
    GfxContext->ProcessDeletionQueue();

    for(auto &Pipeline : this->AllPipelines)
    {
        GfxContext->DestroyPipeline(Pipeline.second);
    }
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

void context::LoadProjectFromFolder(const char *FolderName)
{

    std::vector<std::string> TextureFiles;
    std::vector<std::string> MaterialFiles;
    std::vector<std::string> GeometryFiles;
    std::vector<std::string> ObjectFiles;
    std::vector<std::string> SceneFiles;
    
    namespace fs = std::filesystem;
    for (const auto& entry : fs::directory_iterator(FolderName)) {
        if (fs::is_regular_file(entry)) {
            std::string FileName = entry.path().filename().string();
            std::string Extension = entry.path().extension().string();
            if(Extension == ".mat")
            {
                MaterialFiles.push_back(entry.path().string());
            }
            else if(Extension == ".tex")
            {
                TextureFiles.push_back(entry.path().string());
            }
            else if(Extension == ".geom")
            {
                GeometryFiles.push_back(entry.path().string());
            }
            else if(Extension == ".obj")
            {
                ObjectFiles.push_back(entry.path().string());
            }
            else if(Extension == ".scene")
            {
                SceneFiles.push_back(entry.path().string());
            }
        }
    }

    for (sz i = 0; i < TextureFiles.size(); i++)
    {
        std::shared_ptr<texture> Texture = texture::Deserialize(TextureFiles[i]);
        Project.Textures[Texture->UUID] = Texture;   
    }

    for (sz i = 0; i < MaterialFiles.size(); i++)
    {
        std::shared_ptr<material> Material = material::Deserialize(MaterialFiles[i]);
        Project.Materials[Material->UUID] = Material;   
    }
    
    for (sz i = 0; i < GeometryFiles.size(); i++)
    {
        std::shared_ptr<indexedGeometryBuffers> Geometry = indexedGeometryBuffers::Deserialize(GeometryFiles[i]);
        Project.Geometries[Geometry->UUID] = Geometry;   
    }
    
    for (sz i = 0; i < ObjectFiles.size(); i++)
    {
        std::shared_ptr<object3D> Object = object3D::Deserialize(ObjectFiles[i]);
        Project.Objects[Object->UUID] = Object;   
    }
    
    for (sz i = 0; i < SceneFiles.size(); i++)
    {
        std::shared_ptr<scene> Scene = std::static_pointer_cast<scene>(object3D::Deserialize(SceneFiles[i]));
        Project.Scenes[Scene->UUID] = Scene;  
        if(i==0) this->Scene = Scene;
    }
    
}

void context::SaveProjectToFolder(const char *FolderName)
{
    std::string FolderNameStr = FolderName;
    //Save all the materials
    for (auto &Material : Project.Materials)
    {
        Material.second->Serialize(FolderNameStr + "/" + Material.second->Name + ".mat");
    }
    
    //Save all the textures
    for (auto &Texture : Project.Textures)
    {
        Texture.second->Serialize(FolderNameStr + "/" + Texture.second->Name + ".tex");
    }

    //Save all the geometries
    for (auto &Geometry : Project.Geometries)
    {
        Geometry.second->Serialize(FolderNameStr + "/" + Geometry.second->Name + ".geom");
    }

    //Save all the objects
    for (auto &Object : Project.Objects)
    {
        Object.second->Serialize(FolderNameStr + "/" + Object.second->Name + ".obj");
    }

    //Save all the scenes
    for (auto &Scene : Project.Scenes)
    {
        Scene.second->Serialize(FolderNameStr + "/" + Scene.second->Name + ".scene");
    }
}

context::~context()
{
    Cleanup();
}

}