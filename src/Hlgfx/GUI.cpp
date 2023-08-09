#include "Include/GUI.h"
#include "Include/Context.h"
#include "Include/Scene.h"
#include "Include/Util.h"
#include "Loaders/GLTF.h"
#include <nfd.h>


namespace hlgfx
{

contextGUI::contextGUI(context *Context) : Context(Context) {}
void contextGUI::StartFrame()
{
    IsInteractingGUI = (ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused() || ImGui::IsAnyWindowHovered() || ImGuizmo::IsUsingAny());
}


void contextGUI::DrawGuizmoGUI()
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

void contextGUI::DrawObjectMenu()
{
    if(ImGui::MenuItem("Clone"))
    {
        if(Context->Scene->NodeClicked)
        {

        }
    }
    if(ImGui::MenuItem("Delete"))
    {
        if(Context->Scene->NodeClicked)
        {
            Context->Scene->DeleteObject(Context->Scene->NodeClicked);
            Context->Scene->NodeClicked=nullptr;
        }
    }
}

void contextGUI::AddObjectMenu()
{
    if(ImGui::MenuItem("Empty"))
    {
        std::shared_ptr<hlgfx::object3D> Empty = std::make_shared<hlgfx::object3D>("Empty");
        if(this->Context->Scene->NodeClicked != nullptr)
        {
            this->Context->Scene->NodeClicked->AddObject(Empty);
        }
        else
        {
            this->Context->Scene->AddObject(Empty);
        }
    }
    if(ImGui::MenuItem("Quad"))
    {
        std::shared_ptr<hlgfx::mesh> Mesh = std::make_shared<hlgfx::mesh>();
        Mesh->GeometryBuffers = hlgfx::GetTriangleGeometry();
        Mesh->Material = std::make_shared<hlgfx::unlitMaterial>("New Material");
        if(this->Context->Scene->NodeClicked != nullptr)
        {
            this->Context->Scene->NodeClicked->AddObject(Mesh);
        }
        else
        {
            this->Context->Scene->AddObject(Mesh);
        }
    }
}
void contextGUI::DrawAssetsWindow()
{
    ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Appearing);
    ImGui::Begin("Assets", 0);
    ImGuiTabBarFlags TabBarFlags = ImGuiTabBarFlags_None;
    if (ImGui::BeginTabBar("Assets", TabBarFlags))
    {
        if(ImGui::BeginTabItem("Objects"))
        {
            for (auto &Object : Context->Project.Objects)
            {
                ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
                if(this->SelectedObject3D.get() == Object.second.get()) Flags |= ImGuiTreeNodeFlags_Selected;
                
                if(IsRenaming && this->SelectedObject3D.get() == Object.second.get())
                {
                    //Get cursor pos so we render the text box at the same position as the treenode
                    f32 CursorPos = ImGui::GetCursorPosX();
                    ImGui::TreeNodeEx(Object.second->Name.c_str(), Flags);
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(CursorPos);

                    //Draw text input
                    static char Buffer[64];
                    memcpy(&Buffer, Object.second->Name.data(), Object.second->Name.size());
                    ImGui::SetKeyboardFocusHere();
                    ImGui::PushID(Object.second->UUID.c_str());
                    if(ImGui::InputText("", Buffer, IM_ARRAYSIZE(Buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                    {
                        Object.second->Name = Buffer;
                        this->IsRenaming=false;
                    }
                    ImGui::PopID();
                }
                else
                {
                    ImGui::TreeNodeEx(Object.second->Name.c_str(), Flags);
                }
                
                
                if(ImGui::IsItemClicked())
                {
                    if(this->SelectedObject3D == Object.second && !IsRenaming) this->SelectedObject3D = nullptr;
                    else this->SelectedObject3D = Object.second;
                }
                ImGui::TreePop();
            }
            
            if(this->SelectedObject3D)
            {
                if(ImGui::IsKeyPressed(291)) //F2
                {
                    this->IsRenaming=true;
                }

                ImGui::BeginChild(this->SelectedObject3D->Name.c_str());
                if(ImGui::Button("Add To Scene"))
                {
                    this->Context->Scene->AddObject(this->SelectedObject3D->Clone(false));
                }
                if(ImGui::Button("Duplicate"))
                {
                    Context->AddObjectToProject(this->SelectedObject3D->Clone(false));
                }
                if(ImGui::Button("Delete"))
                {
                    Context->RemoveObjectFromProject(this->SelectedObject3D);
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
                    Context->AddObjectToProject(Mesh);
                }                    
            }

            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Materials"))
        {
            if (ImGui::Button("Add New"))
            {
                Context->AddMaterialToProject(std::make_shared<unlitMaterial>("New Material"));
            }

            for (auto &Material : Context->Project.Materials)
            {
                ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
                if(this->SelectedMaterial.get() == Material.second.get()) Flags |= ImGuiTreeNodeFlags_Selected;
                
                if(IsRenaming && this->SelectedMaterial.get() == Material.second.get())
                {
                    //Get cursor pos so we render the text box at the same position as the treenode
                    f32 CursorPos = ImGui::GetCursorPosX();
                    ImGui::TreeNodeEx(Material.second->Name.c_str(), Flags);
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(CursorPos);

                    //Draw text input
                    static char Buffer[64];
                    memcpy(&Buffer, Material.second->Name.data(), Material.second->Name.size());
                    ImGui::SetKeyboardFocusHere();
                    ImGui::PushID(Material.second->UUID.c_str());
                    if(ImGui::InputText("", Buffer, IM_ARRAYSIZE(Buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                    {
                        Material.second->Name = Buffer;
                        this->IsRenaming=false;
                    }
                    ImGui::PopID();
                }
                else
                {
                    ImGui::TreeNodeEx(Material.second->Name.c_str(), Flags);
                }                

                if(ImGui::IsItemClicked())
                {
                    if(this->SelectedMaterial == Material.second && !this->IsRenaming) this->SelectedMaterial = nullptr;
                    else this->SelectedMaterial = Material.second;
                }

                ImGui::TreePop();
            }

            if(this->SelectedMaterial)
            {
                if(ImGui::IsKeyPressed(291)) //F2
                {
                    this->IsRenaming=true;
                }

                ImGui::BeginChild("Material");
                if(ImGui::Button("Duplicate"))
                {
                    Context->AddMaterialToProject(this->SelectedMaterial->Clone());
                }
                if(ImGui::Button("Delete"))
                {
                    Context->RemoveMaterialFromProject(this->SelectedMaterial);
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
                    Context->AddTextureToProject(Texture);
                }                            
            }
            for (auto &Texture : Context->Project.Textures)
            {
                ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
                if(this->SelectedTexture.get() == Texture.second.get()) Flags |= ImGuiTreeNodeFlags_Selected;

                gfx::image *Image = gfx::context::Get()->GetImage(Texture.second->Handle);
                ImGui::Image(Image->GetImGuiID(), ImVec2(40, 40));
                ImGui::SameLine();
                if(IsRenaming && this->SelectedTexture.get() == Texture.second.get())
                {
  
                    //Get cursor pos so we render the text box at the same position as the treenode
                    f32 CursorPos = ImGui::GetCursorPosX();
                    ImGui::TreeNodeEx(Texture.second->Name.c_str(), Flags);
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(CursorPos);

                    //Draw text input
                    static char Buffer[64];
                    memcpy(&Buffer, Texture.second->Name.data(), Texture.second->Name.size());
                    ImGui::SetKeyboardFocusHere();
                    ImGui::PushID(Texture.second->UUID.c_str());
                    if(ImGui::InputText("", Buffer, IM_ARRAYSIZE(Buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                    {
                        Texture.second->Name = Buffer;
                        this->IsRenaming=false;
                    }
                    ImGui::PopID();
                }
                else
                {
                    ImGui::TreeNodeEx(Texture.second->Name.c_str(), Flags);
                }      

                if(ImGui::IsItemClicked())
                {
                    if(this->SelectedTexture == Texture.second && !IsRenaming) this->SelectedTexture =nullptr;
                    else this->SelectedTexture = Texture.second;
                }
                ImGui::TreePop();
            }
            ImGui::Separator();
            if(this->SelectedTexture!=nullptr)
            {
                if(ImGui::IsKeyPressed(291)) //F2
                {
                    this->IsRenaming=true;
                }
                ImGui::BeginChild(this->SelectedTexture->Name.c_str());
                gfx::image *Image = gfx::context::Get()->GetImage(this->SelectedTexture->Handle);
                ImGui::Image(Image->GetImGuiID(), ImVec2(256, 256));
                ImGui::Text("Size : %ux%u", Image->Extent.Width, Image->Extent.Height);
                ImGui::Text("Channel Count : %u", Image->ChannelCount);
                ImGui::Text("Mip Levels : %u", Image->MipLevelCount);
                ImGui::Text("Format : %s", gfx::FormatToString(Image->Format));

                if(ImGui::Button("Delete"))
                {
                    Context->RemoveTextureFromProject(this->SelectedTexture);
                }
                if(ImGui::Button("Duplicate"))
                {
                    Context->AddTextureToProject(this->SelectedTexture->Clone());
                }
                ImGui::EndChild();
            }

            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Geometries"))
        {
            for (auto &Geometry : Context->Project.Geometries)
            {
                ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
                if(this->SelectedIndexedGeometryBuffers.get() == Geometry.second.get()) Flags |= ImGuiTreeNodeFlags_Selected;
                

                if(IsRenaming && this->SelectedIndexedGeometryBuffers.get() == Geometry.second.get())
                {
                    //Get cursor pos so we render the text box at the same position as the treenode
                    f32 CursorPos = ImGui::GetCursorPosX();
                    ImGui::TreeNodeEx(Geometry.second->Name.c_str(), Flags);
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(CursorPos);

                    //Draw text input
                    static char Buffer[64];
                    memcpy(&Buffer, Geometry.second->Name.data(), Geometry.second->Name.size());
                    ImGui::SetKeyboardFocusHere();
                    ImGui::PushID(Geometry.second->UUID.c_str());
                    if(ImGui::InputText("", Buffer, IM_ARRAYSIZE(Buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                    {
                        Geometry.second->Name = Buffer;
                        this->IsRenaming=false;
                    }
                    ImGui::PopID();
                }
                else
                {
                    ImGui::TreeNodeEx(Geometry.second->Name.c_str(), Flags);
                }  

                if(ImGui::IsItemClicked())
                {
                    if(this->SelectedIndexedGeometryBuffers == Geometry.second && !IsRenaming) this->SelectedIndexedGeometryBuffers =nullptr;
                    else this->SelectedIndexedGeometryBuffers = Geometry.second;                    
                }
                ImGui::TreePop();
            }
            ImGui::Separator();
            if(SelectedIndexedGeometryBuffers!=nullptr)
            {
                if(ImGui::IsKeyPressed(291)) //F2
                {
                    this->IsRenaming=true;
                }

                ImGui::BeginChild(SelectedIndexedGeometryBuffers->Name.c_str());
                ImGui::Text("Start Index %u", SelectedIndexedGeometryBuffers->Start);
                ImGui::Text("Index Count %u", SelectedIndexedGeometryBuffers->Count);
                ImGui::Text("Vertex Count %u", SelectedIndexedGeometryBuffers->VertexData.size());

                ImGui::EndChild();
            }
            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Scenes"))
        {
            for (auto &Scene : Context->Project.Scenes)
            {
                ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
                if(this->SelectedScene.get() == Scene.second.get()) Flags |= ImGuiTreeNodeFlags_Selected;
                
                if(IsRenaming && this->SelectedScene.get() == Scene.second.get())
                {
                    //Get cursor pos so we render the text box at the same position as the treenode
                    f32 CursorPos = ImGui::GetCursorPosX();
                    ImGui::TreeNodeEx(Scene.second->Name.c_str(), Flags);
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(CursorPos);

                    //Draw text input
                    static char Buffer[64];
                    memcpy(&Buffer, Scene.second->Name.data(), Scene.second->Name.size());
                    ImGui::SetKeyboardFocusHere();
                    ImGui::PushID(Scene.second->UUID.c_str());
                    if(ImGui::InputText("", Buffer, IM_ARRAYSIZE(Buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                    {
                        Scene.second->Name = Buffer;
                        this->IsRenaming=false;
                    }
                    ImGui::PopID();
                }
                else
                {
                    ImGui::TreeNodeEx(Scene.second->Name.c_str(), Flags);
                }  

                
                if(ImGui::IsItemClicked())
                {
                    if(this->SelectedScene == Scene.second && !IsRenaming) this->SelectedScene =nullptr;
                    else this->SelectedScene = Scene.second;                            
                }
                ImGui::TreePop();
            }

            if(this->SelectedScene != nullptr)
            {
                
                if(ImGui::IsKeyPressed(291)) //F2
                {
                    this->IsRenaming=true;
                }

                if(ImGui::Button("Open"))
                {
                    Context->Scene = this->SelectedScene;
                }
                if(ImGui::Button("Duplicate"))
                {
                    std::shared_ptr<scene> Duplicate = std::static_pointer_cast<scene>(this->Context->Scene->Clone(false));
                    Context->AddSceneToProject(Duplicate);
                    Context->Scene = this->SelectedScene;
                }
                if(ImGui::Button("Delete"))
                {
                    if(Context->Project.Scenes.size() > 1)
                    {
                        Context->Project.Scenes.erase(this->Context->Scene->UUID);
                        for (auto &Scene : Context->Project.Scenes)
                        {
                            Context->Scene = Scene.second;
                            this->SelectedScene = Context->Scene;
                            break;
                        }
                    }
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void contextGUI::DrawMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if(ImGui::MenuItem("New Scene"))
            {
                std::shared_ptr<scene> NewScene = std::make_shared<scene>("New_Scene_" + std::to_string(Context->Project.Scenes.size()));
                Context->AddSceneToProject(NewScene);
            }
            if(ImGui::MenuItem("Import GLTF"))
            {
                nfdchar_t *OutPath = NULL;
                nfdresult_t Result = NFD_OpenDialog( NULL, NULL, &OutPath );
                if ( Result == NFD_OKAY ) {
                    std::shared_ptr<hlgfx::object3D> Mesh = hlgfx::loaders::gltf::Load(OutPath);
                    Context->AddObjectToProject(Mesh);
                }                
            }
            ImGui::Separator();
            if(ImGui::MenuItem("Save Project"))
            {
                nfdchar_t *OutPath = NULL;
                nfdresult_t Result = NFD_SaveDialog(NULL, NULL, &OutPath );
                if ( Result == NFD_OKAY ) {
                    Context->SaveProjectToFile(OutPath);
                }
            }
            if(ImGui::MenuItem("Load Project"))
            {
                nfdchar_t *OutPath = NULL;
                nfdresult_t Result = NFD_OpenDialog(NULL, NULL, &OutPath );
                if ( Result == NFD_OKAY ) {
                    Context->LoadProjectFromFile(OutPath);
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


void contextGUI::DrawGUI()
{
    DrawGuizmoGUI();
    DrawMainMenuBar();
    if(ShowAssetsWindow) DrawAssetsWindow();
    this->Context->Scene->DrawGUI();
}

}