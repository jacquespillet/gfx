#include "Include/GUI.h"
#include "Include/Context.h"
#include "Include/Scene.h"
#include "Include/Util.h"
#include "Include/Light.h"
#include "Loaders/GLTF.h"
#include "Loaders/Assimp.h"
#include <nfd.h>
#include <glm/ext.hpp>
#include <imgui.h>

namespace hlgfx
{

contextGUI::contextGUI(context *Context) : Context(Context) {}

void contextGUI::StartFrame()
{
    IsInteractingGUI = (ImGui::IsAnyItemHovered() || ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused()  || ImGuizmo::IsUsingAny());
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
    if(ImGui::MenuItem("Duplicate"))
    {
        if(Context->Scene->SceneGUI->NodeClicked)
        {

        }
    }
    if(ImGui::MenuItem("Delete"))
    {
        if(Context->Scene->SceneGUI->NodeClicked)
        {
            Context->Scene->DeleteObject(Context->Scene->SceneGUI->NodeClicked);
            Context->Scene->SceneGUI->NodeClicked=nullptr;
        }
    }
}

void contextGUI::AddObjectMenu()
{
    std::shared_ptr<object3D> ObjectToAdd;
    std::shared_ptr<object3D> ParentObject;
    b8 Add = false;
    if(ImGui::MenuItem("Empty"))
    {
        Add = true;
        ObjectToAdd = std::make_shared<hlgfx::object3D>("Empty");
        if(this->Context->Scene->SceneGUI->NodeClicked != nullptr)
            ParentObject = this->Context->Scene->SceneGUI->NodeClicked;
        else
            ParentObject = this->Context->Scene;
    }
    if(ImGui::BeginMenu("Light"))
    {
        if(ImGui::MenuItem("Point"))
        {
            Add = true;
            ObjectToAdd = std::make_shared<light>("Point Light", light::lightType::Point);
            std::shared_ptr<light> Light = std::static_pointer_cast<light>(ObjectToAdd);
            if(this->Context->Scene->SceneGUI->NodeClicked != nullptr)
                ParentObject = this->Context->Scene->SceneGUI->NodeClicked;
            else
                ParentObject = this->Context->Scene;            
        }
        if(ImGui::MenuItem("Directional"))
        {
            Add = true;
            ObjectToAdd = std::make_shared<light>("Directional Light", light::lightType::Directional);
            std::shared_ptr<light> Light = std::static_pointer_cast<light>(ObjectToAdd);
            if(this->Context->Scene->SceneGUI->NodeClicked != nullptr)
                ParentObject = this->Context->Scene->SceneGUI->NodeClicked;
            else
                ParentObject = this->Context->Scene;    

            
        }
        if(ImGui::MenuItem("Area"))
        {

        }
        if(ImGui::MenuItem("Spot"))
        {

        }
        ImGui::EndMenu();
    }
    if(ImGui::MenuItem("Quad"))
    {
        Add = true;
        std::shared_ptr<hlgfx::mesh> Mesh = std::make_shared<mesh>("Quad");
        Mesh->GeometryBuffers = context::Get()->Quad;
        Mesh->Material = context::Get()->NoMaterial->Clone();
        context::Get()->AddMaterialToProject(Mesh->Material);
        
        ObjectToAdd = Mesh;
        if(this->Context->Scene->SceneGUI->NodeClicked != nullptr)
            ParentObject = this->Context->Scene->SceneGUI->NodeClicked;
        else
            ParentObject = this->Context->Scene;
        
    }
    if(ImGui::MenuItem("Cube"))
    {
        Add = true;
        std::shared_ptr<hlgfx::mesh> Mesh = std::make_shared<mesh>("Cube");
        Mesh->GeometryBuffers = context::Get()->Cube;
        Mesh->Material = context::Get()->NoMaterial->Clone();
        context::Get()->AddMaterialToProject(Mesh->Material);
        
        ObjectToAdd = Mesh;
        if(this->Context->Scene->SceneGUI->NodeClicked != nullptr)
            ParentObject = this->Context->Scene->SceneGUI->NodeClicked;
        else
            ParentObject = this->Context->Scene;
    }
    if(ImGui::MenuItem("Sphere"))
    {
        Add = true;
        std::shared_ptr<hlgfx::mesh> Mesh = std::make_shared<mesh>("Sphere");
        Mesh->GeometryBuffers = context::Get()->Sphere;
        Mesh->Material = context::Get()->NoMaterial->Clone();
        context::Get()->AddMaterialToProject(Mesh->Material);
        
        ObjectToAdd = Mesh;
        if(this->Context->Scene->SceneGUI->NodeClicked != nullptr)
            ParentObject = this->Context->Scene->SceneGUI->NodeClicked;
        else
            ParentObject = this->Context->Scene;
    }
    if(ImGui::MenuItem("Cone"))
    {
        Add = true;
        std::shared_ptr<hlgfx::mesh> Mesh = std::make_shared<mesh>("Cone");
        Mesh->GeometryBuffers = context::Get()->Cone;
        Mesh->Material = context::Get()->NoMaterial->Clone();
        context::Get()->AddMaterialToProject(Mesh->Material);
        
        ObjectToAdd = Mesh;
        if(this->Context->Scene->SceneGUI->NodeClicked != nullptr)
            ParentObject = this->Context->Scene->SceneGUI->NodeClicked;
        else
            ParentObject = this->Context->Scene;
    }
    if(ImGui::MenuItem("Capsule"))
    {
        Add = true;
        std::shared_ptr<hlgfx::mesh> Mesh = std::make_shared<mesh>("Capsule");
        Mesh->GeometryBuffers = context::Get()->Capsule;
        Mesh->Material = context::Get()->NoMaterial->Clone();
        context::Get()->AddMaterialToProject(Mesh->Material);
        
        ObjectToAdd = Mesh;
        if(this->Context->Scene->SceneGUI->NodeClicked != nullptr)
            ParentObject = this->Context->Scene->SceneGUI->NodeClicked;
        else
            ParentObject = this->Context->Scene;
    }
    if(ImGui::MenuItem("Cylinder"))
    {
        Add = true;
        std::shared_ptr<hlgfx::mesh> Mesh = std::make_shared<mesh>("Cylinder");
        Mesh->GeometryBuffers = context::Get()->Cylinder;
        Mesh->Material = context::Get()->NoMaterial->Clone();
        context::Get()->AddMaterialToProject(Mesh->Material);
        
        ObjectToAdd = Mesh;
        if(this->Context->Scene->SceneGUI->NodeClicked != nullptr)
            ParentObject = this->Context->Scene->SceneGUI->NodeClicked;
        else
            ParentObject = this->Context->Scene;
    }  

    if(Add)
    {
        ParentObject->AddObject(ObjectToAdd);
    }
}

void contextGUI::DrawCameraWindow()
{
    ImGui::SetNextWindowSize(ImVec2(500, 500), ImGuiCond_Appearing);
    ImGui::Begin("Camera", 0);
    ImGuiTabBarFlags TabBarFlags = ImGuiTabBarFlags_None;

    b8 Changed = false;
    Changed |= ImGui::DragFloat("Far Clip", &this->Context->CurrentCamera->Data.FarClip, 1, 1);

    if(Changed)
    {
        this->Context->CurrentCamera->RecalculateMatrices();
    }
    
    b8 LightsChanged = false;
    LightsChanged |= ImGui::DragFloat("Shadow Bias", &this->Context->Scene->SceneBufferData.ShadowBias, 0.001f);

    if(LightsChanged)
    {
        this->Context->Scene->UpdateLight(0);
    }
    

    ImGui::End();
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
                if(this->SelectedObject3D.get() == Object.get()) Flags |= ImGuiTreeNodeFlags_Selected;
                
                if(IsRenaming && this->SelectedObject3D.get() == Object.get())
                {
                    //Get cursor pos so we render the text box at the same position as the treenode
                    f32 CursorPos = ImGui::GetCursorPosX();
                    ImGui::TreeNodeEx(Object->Name.c_str(), Flags);
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(CursorPos);

                    //Draw text input
                    static char Buffer[64];
                    memcpy(&Buffer, Object->Name.data(), Object->Name.size());
                    ImGui::SetKeyboardFocusHere();
                    ImGui::PushID(Object->ID);
                    if(ImGui::InputText("", Buffer, IM_ARRAYSIZE(Buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                    {
                        Object->Name = Buffer;
                        this->IsRenaming=false;
                    }
                    ImGui::PopID();
                }
                else
                {
                    ImGui::TreeNodeEx(Object->Name.c_str(), Flags);
                }
                
                
                if(ImGui::IsItemClicked())
                {
                    if(this->SelectedObject3D == Object && !IsRenaming) this->SelectedObject3D = nullptr;
                    else this->SelectedObject3D = Object;

                    context::Get()->Scene->SceneGUI->NodeClicked = nullptr;
                }
                ImGui::TreePop();
            }
            
            if(this->SelectedObject3D)
            {
                if(ImGui::IsKeyPressed(ImGuiKey_F2)) //F2
                {
                    this->IsRenaming=true;
                }

                ImGui::BeginChild(this->SelectedObject3D->Name.c_str());
                if(ImGui::Button("Add To Scene"))
                {
                    this->Context->Scene->AddObject(this->SelectedObject3D->Clone(false));
                }
                if(ImGui::Button("Duplicate") || (context::Get()->CtrlPressed && ImGui::IsKeyPressed(ImGuiKey_D)))
                {
                    Context->AddObjectToProject(this->SelectedObject3D->Clone(false));
                }
                if(ImGui::Button("Delete") || ImGui::IsKeyPressed(ImGuiKey_Delete))
                {
                    Context->RemoveObjectFromProject(this->SelectedObject3D);
                    this->SelectedObject3D=nullptr;
                }
                ImGui::EndChild();
            }
            
            ImGui::Separator();
            
            if(ImGui::Button("Import"))
            {
                nfdchar_t *OutPath = NULL;
                nfdresult_t Result = NFD_OpenDialog( NULL, NULL, &OutPath );
                if ( Result == NFD_OKAY ) {
                    object3D::AddToScene = false;
                    std::shared_ptr<hlgfx::object3D> Mesh = hlgfx::loaders::gltf::Load(OutPath);
                    // Context->AddObjectToProject(Mesh);
                    object3D::AddToScene = true;
                }                    
            }

            ImGui::EndTabItem();
        }
        if(ImGui::BeginTabItem("Materials"))
        {
            if (ImGui::Button("Add New"))
            {
                Context->AddMaterialToProject(std::make_shared<pbrMaterial>("New Material"));
            }

            for (auto &Material : Context->Project.Materials)
            {
                ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
                if(this->SelectedMaterial.get() == Material.get()) Flags |= ImGuiTreeNodeFlags_Selected;
                
                if(IsRenaming && this->SelectedMaterial.get() == Material.get())
                {
                    //Get cursor pos so we render the text box at the same position as the treenode
                    f32 CursorPos = ImGui::GetCursorPosX();
                    ImGui::TreeNodeEx(Material->Name.c_str(), Flags);
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(CursorPos);

                    //Draw text input
                    static char Buffer[64];
                    memcpy(&Buffer, Material->Name.data(), Material->Name.size());
                    ImGui::SetKeyboardFocusHere();
                    ImGui::PushID(Material->ID);
                    if(ImGui::InputText("", Buffer, IM_ARRAYSIZE(Buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                    {
                        Material->Name = Buffer;
                        this->IsRenaming=false;
                    }
                    ImGui::PopID();
                }
                else
                {
                    ImGui::TreeNodeEx(Material->Name.c_str(), Flags);
                }                

                if(ImGui::IsItemClicked())
                {
                    if(this->SelectedMaterial == Material && !this->IsRenaming) this->SelectedMaterial = nullptr;
                    else this->SelectedMaterial = Material;
                    context::Get()->Scene->SceneGUI->NodeClicked = nullptr;
                }

                ImGui::TreePop();
            }

            if(this->SelectedMaterial)
            {
                if(ImGui::IsKeyPressed(ImGuiKey_F2)) //F2
                {
                    this->IsRenaming=true;
                }

                ImGui::BeginChild("Material");
                if(ImGui::Button("Duplicate") || (context::Get()->CtrlPressed && ImGui::IsKeyPressed(ImGuiKey_D)))
                {
                    Context->AddMaterialToProject(this->SelectedMaterial->Clone());
                }
                if(ImGui::Button("Delete") || ImGui::IsKeyPressed(ImGuiKey_Delete))
                {
                    Context->RemoveMaterialFromProject(this->SelectedMaterial);
                    this->SelectedMaterial = nullptr;
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
                if(this->SelectedTexture.get() == Texture.get()) Flags |= ImGuiTreeNodeFlags_Selected;

                gfx::image *Image = gfx::context::Get()->GetImage(Texture->Handle);
                ImGui::Image(Image->GetImGuiID(), ImVec2(40, 40));
                ImGui::SameLine();
                if(IsRenaming && this->SelectedTexture.get() == Texture.get())
                {
  
                    //Get cursor pos so we render the text box at the same position as the treenode
                    f32 CursorPos = ImGui::GetCursorPosX();
                    ImGui::TreeNodeEx(Texture->Name.c_str(), Flags);
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(CursorPos);

                    //Draw text input
                    static char Buffer[64];
                    memcpy(&Buffer, Texture->Name.data(), Texture->Name.size());
                    ImGui::SetKeyboardFocusHere();
                    ImGui::PushID(Texture->ID);
                    if(ImGui::InputText("", Buffer, IM_ARRAYSIZE(Buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                    {
                        Texture->Name = Buffer;
                        this->IsRenaming=false;
                    }
                    ImGui::PopID();
                }
                else
                {
                    ImGui::TreeNodeEx(Texture->Name.c_str(), Flags);
                }      

                if(ImGui::IsItemClicked())
                {
                    if(this->SelectedTexture == Texture && !IsRenaming) this->SelectedTexture =nullptr;
                    else this->SelectedTexture = Texture;
                    context::Get()->Scene->SceneGUI->NodeClicked = nullptr;
                }
                ImGui::TreePop();
            }
            ImGui::Separator();
            if(this->SelectedTexture!=nullptr)
            {
                if(ImGui::IsKeyPressed(ImGuiKey_F2)) //F2
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

                if(ImGui::Button("Delete") || ImGui::IsKeyPressed(ImGuiKey_Delete))
                {
                    Context->RemoveTextureFromProject(this->SelectedTexture);
                    this->SelectedTexture=nullptr;
                }
                if(ImGui::Button("Duplicate") || (context::Get()->CtrlPressed && ImGui::IsKeyPressed(ImGuiKey_D)))
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
                if(this->SelectedIndexedGeometryBuffers.get() == Geometry.get()) Flags |= ImGuiTreeNodeFlags_Selected;
                

                if(IsRenaming && this->SelectedIndexedGeometryBuffers.get() == Geometry.get())
                {
                    //Get cursor pos so we render the text box at the same position as the treenode
                    f32 CursorPos = ImGui::GetCursorPosX();
                    ImGui::TreeNodeEx(Geometry->Name.c_str(), Flags);
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(CursorPos);

                    //Draw text input
                    static char Buffer[64];
                    memcpy(&Buffer, Geometry->Name.data(), Geometry->Name.size());
                    ImGui::SetKeyboardFocusHere();
                    ImGui::PushID(Geometry->ID);
                    if(ImGui::InputText("", Buffer, IM_ARRAYSIZE(Buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                    {
                        Geometry->Name = Buffer;
                        this->IsRenaming=false;
                    }
                    ImGui::PopID();
                }
                else
                {
                    ImGui::TreeNodeEx(Geometry->Name.c_str(), Flags);
                }  

                if(ImGui::IsItemClicked())
                {
                    if(this->SelectedIndexedGeometryBuffers == Geometry && !IsRenaming) this->SelectedIndexedGeometryBuffers =nullptr;
                    else this->SelectedIndexedGeometryBuffers = Geometry;    
                    context::Get()->Scene->SceneGUI->NodeClicked = nullptr;                
                }
                ImGui::TreePop();
            }
            ImGui::Separator();
            if(SelectedIndexedGeometryBuffers!=nullptr)
            {
                if(ImGui::IsKeyPressed(ImGuiKey_F2)) //F2
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
                if(this->SelectedScene.get() == Scene.get()) Flags |= ImGuiTreeNodeFlags_Selected;
                
                if(IsRenaming && this->SelectedScene.get() == Scene.get())
                {
                    //Get cursor pos so we render the text box at the same position as the treenode
                    f32 CursorPos = ImGui::GetCursorPosX();
                    ImGui::TreeNodeEx(Scene->Name.c_str(), Flags);
                    ImGui::SameLine();
                    ImGui::SetCursorPosX(CursorPos);

                    //Draw text input
                    static char Buffer[64];
                    memcpy(&Buffer, Scene->Name.data(), Scene->Name.size());
                    ImGui::SetKeyboardFocusHere();
                    ImGui::PushID(Scene->ID);
                    if(ImGui::InputText("", Buffer, IM_ARRAYSIZE(Buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
                    {
                        Scene->Name = Buffer;
                        this->IsRenaming=false;
                    }
                    ImGui::PopID();
                }
                else
                {
                    ImGui::TreeNodeEx(Scene->Name.c_str(), Flags);
                }  

                
                if(ImGui::IsItemClicked())
                {
                    if(this->SelectedScene == Scene && !IsRenaming) this->SelectedScene =nullptr;
                    else this->SelectedScene = Scene;          
                    context::Get()->Scene->SceneGUI->NodeClicked = nullptr;                  
                }
                ImGui::TreePop();
            }

            if(this->SelectedScene != nullptr)
            {
                
                if(ImGui::IsKeyPressed(ImGuiKey_F2)) //F2
                {
                    this->IsRenaming=true;
                }

                if(ImGui::Button("Open"))
                {
                    Context->Scene = this->SelectedScene;
                }
                if(ImGui::Button("Duplicate") || (context::Get()->CtrlPressed && ImGui::IsKeyPressed(ImGuiKey_D)))
                {
                    std::shared_ptr<scene> Duplicate = std::static_pointer_cast<scene>(this->Context->Scene->Clone(false));
                    Context->AddSceneToProject(Duplicate);
                    Context->Scene = this->SelectedScene;
                }
                if(ImGui::Button("Delete") || ImGui::IsKeyPressed(ImGuiKey_Delete))
                {
#if 0
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
                    this->SelectedScene = nullptr;
#endif
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}


void contextGUI::SaveAsProject()
{
    nfdchar_t *OutPath = NULL;
    nfdresult_t Result = NFD_SaveDialog(NULL, NULL, &OutPath );
    if ( Result == NFD_OKAY ) {
        Context->SaveProjectToFile(OutPath);
        this->ProjectFile = OutPath;
    }
}

void contextGUI::SaveProject()
{
    if(this->ProjectFile == "") SaveAsProject();
    else Context->SaveProjectToFile(ProjectFile.c_str());
}

void contextGUI::OpenProject()
{
    nfdchar_t *OutPath = NULL;
    nfdresult_t Result = NFD_OpenDialog(NULL, NULL, &OutPath );
    if ( Result == NFD_OKAY ) {
        Context->NewProject(); //This clears the project
        Context->LoadProjectFromFile(OutPath);
        this->ProjectFile = OutPath;
    }     
}

void contextGUI::NewProject()
{
    nfdchar_t *OutPath = NULL;
    nfdresult_t Result = NFD_SaveDialog(NULL, NULL, &OutPath );
    if ( Result == NFD_OKAY ) {
        Context->SaveProjectToFile(OutPath);
        Context->NewProject();
        this->ProjectFile = "";
    }
}


void contextGUI::DrawMainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if(GFX_API==GFX_VK)
        {
            ImGui::Text("Mini Engine (Vulkan)");
        }
        else if(GFX_API==GFX_GL)
        {
            ImGui::Text("Mini Engine (OpenGL)");
        }
        else if(GFX_API==GFX_D3D11)
        {
            ImGui::Text("Mini Engine (Direct3D 11)");
        }
        else if(GFX_API==GFX_D3D12)
        {
            ImGui::Text("Mini Engine (Direct3D 12)");
    }
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
                    object3D::AddToScene = false;
                    std::shared_ptr<hlgfx::object3D> Mesh = hlgfx::loaders::gltf::Load(OutPath);
                    Context->AddObjectToProject(Mesh);
                    object3D::AddToScene = true;
                }                
            }
            if(ImGui::MenuItem("Import Other"))
            {
                nfdchar_t *OutPath = NULL;
                nfdresult_t Result = NFD_OpenDialog( NULL, NULL, &OutPath );
                if ( Result == NFD_OKAY ) {
                    object3D::AddToScene = false;
                    std::shared_ptr<hlgfx::object3D> Mesh = hlgfx::loaders::assimp::Load(OutPath);
                    Context->AddObjectToProject(Mesh);
                    object3D::AddToScene = true;
                }                
            }
            ImGui::Separator();
            if(ImGui::MenuItem("New Project", "Ctrl + N"))
            {
                NewProject();
            }
            if(ImGui::MenuItem("Save Project", "Ctrl + S"))
            {
                if(this->ProjectFile != "")
                {
                    SaveProject();
                }
                else
                {
                    SaveAsProject();
                }
            }
            if(ImGui::MenuItem("Save Project As", "Ctrl + S"))
            {
                SaveAsProject();
            }
            if(ImGui::MenuItem("Open Project", "Ctrl + O"))
            {
                OpenProject();
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
        if(ImGui::MenuItem("Camera"))
        {
            this->ShowCameraWindow = !this->ShowCameraWindow;
        }
        ImGui::EndMainMenuBar();
    }
}


void contextGUI::DrawGUI()
{
    DrawGuizmoGUI();
    DrawMainMenuBar();
    if(ShowAssetsWindow) DrawAssetsWindow();
    if(ShowCameraWindow) DrawCameraWindow();
    this->Context->Scene->DrawGUI();

    if(Context->CtrlPressed)
    {
        if(ImGui::IsKeyPressed(ImGuiKey_S))
            SaveProject();
        else if(ImGui::IsKeyPressed(ImGuiKey_N))
            NewProject();
        else if(ImGui::IsKeyPressed(ImGuiKey_O))
            OpenProject();
    }
}

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

sceneGUI::sceneGUI(scene *Scene) : Scene(Scene) {}


void sceneGUI::DrawGUI()
{
    u32 WindowWidth = context::Get()->Width;
    u32 WindowHeight = context::Get()->Height;

    ImGui::SetNextWindowPos(ImVec2(WindowWidth - GuiWidth, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(GuiWidth, WindowHeight), ImGuiCond_Always);
    
    ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("RightPanel", nullptr, WindowFlags);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1,1,1,1));

    ImGui::BeginChild("Scene", ImVec2(GuiWidth, 300));
    if(ImGui::BeginPopupContextWindow("Scene"))
    {
        if(ImGui::BeginMenu("Add Object"))
        {
            context::Get()->GUI->AddObjectMenu();
            ImGui::EndMenu();
        }
        ImGui::EndPopup();
    }
    DrawSceneGUI();
    ImGui::EndChild();  
    ImGui::PopStyleColor(1);


    if(NodeClicked != nullptr && NodeClicked != context::Get()->Scene)
    {
        NodeClicked->DrawGUI();
    }
    else if (NodeClicked == context::Get()->Scene)
    {  

    }

    this->GuiWidth = ImGui::GetWindowSize().x;
    ImGui::End();
}




void sceneGUI::DrawNodeChildren(hlgfx::object3D *Object)
{
    if(ImGui::IsKeyPressed(ImGuiKey_F2))
    {
        this->IsRenaming=true;
    }
    //For each children, draw it
    static ImGuiTreeNodeFlags BaseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_NavLeftJumpsBackHere;
    for(size_t i=0; i<Object->Children.size(); i++)
    {
        ImGuiTreeNodeFlags NodeFlags = BaseFlags;
        
        int NumChildren = Object->Children[i]->Children.size();
        if(NumChildren == 0) NodeFlags |= ImGuiTreeNodeFlags_Leaf;
        if(Object->Children[i] == NodeClicked) NodeFlags |= ImGuiTreeNodeFlags_Selected;

        bool NodeOpen=false;
        //If renaming
        if(this->IsRenaming && Object->Children[i] == NodeClicked) 
        {
            //Get cursor pos so we render the text box at the same position as the treenode
            f32 CursorPos = ImGui::GetCursorPosX();
            NodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, NodeFlags, Object->Children[i]->Name.c_str(), i);

            ImGui::SameLine();
            ImGui::SetCursorPosX(CursorPos);
            ImGui::PushID(i);

            //Draw text input
            static char Buffer[64];
            memcpy(&Buffer, Object->Children[i]->Name.data(), Object->Children[i]->Name.size());
            ImGui::SetKeyboardFocusHere();
            if(ImGui::InputText("", Buffer, IM_ARRAYSIZE(Buffer), ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_AutoSelectAll))
            {
                Object->Children[i]->Name = Buffer;
                this->IsRenaming=false;
            }
            ImGui::PopID();
        }
        else
        {
            NodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, NodeFlags, Object->Children[i]->Name.c_str(), i);
        }

        if(ImGui::IsItemFocused())
        {
            NodeClicked = Object->Children[i];
        }
        if(ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
        {
            if(NodeClicked == Object->Children[i] && ImGui::IsItemClicked(0) && !this->IsRenaming) NodeClicked = nullptr;
            else NodeClicked = Object->Children[i];
        }

        ImGui::PushID(i);
        if (ImGui::BeginPopupContextItem("Actions"))
        {
            context::Get()->GUI->DrawObjectMenu();
            if(ImGui::BeginMenu("Add"))
            {
                context::Get()->GUI->AddObjectMenu();
                ImGui::EndMenu();
            }
            if(ImGui::MenuItem("Duplicate"))
            {
                std::shared_ptr<object3D> Duplicate = this->NodeClicked->Clone(false);
                Scene->AddObject(Duplicate);
            }
            if(ImGui::MenuItem("Rename"))
            {
                this->IsRenaming=true;
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();



        //Drag and drop
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
        {
            ImGui::SetDragDropPayload("DND_DEMO_CELL", &Object->Children[i], sizeof(std::shared_ptr<hlgfx::object3D>));    // Set payload to carry the index of our item (could be anything)
            ImGui::Text("Move");
            ImGui::EndDragDropSource();
        }
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
            {
                std::shared_ptr<hlgfx::object3D> Payload = *(std::shared_ptr<hlgfx::object3D>*)payload->Data;
                Payload->SetParent(Object->Children[i]);
            }
            ImGui::EndDragDropTarget();
        }			

        if(NodeOpen)
        {
            if(i < Object->Children.size())
            {
                DrawNodeChildren(Object->Children[i].get());
            }
            ImGui::TreePop();
        }
    }
}

void sceneGUI::DrawSceneGUI()
{
    std::shared_ptr<scene> ScenePtr = context::Get()->Scene;

    static ImGuiTreeNodeFlags BaseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_DefaultOpen;
    int NumChildren = Scene->Children.size();
    if(NumChildren == 0) BaseFlags |= ImGuiTreeNodeFlags_Leaf;

    bool NodeOpen = ImGui::TreeNodeEx("Scene", BaseFlags, Scene->Name.c_str());

    if(ImGui::IsItemClicked())
    {
        NodeClicked = ScenePtr;
    }

    // Drag and drop
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        ImGui::SetDragDropPayload("DND_DEMO_CELL", &ScenePtr, sizeof(std::shared_ptr<hlgfx::object3D>));
        ImGui::Text("Move");
        ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget())
    {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
        {
            std::shared_ptr<hlgfx::object3D> Payload = *(std::shared_ptr<hlgfx::object3D>*)payload->Data;
            Payload->SetParent(ScenePtr);  
        }
        ImGui::EndDragDropTarget();
    }

    if(NodeOpen)
    {
        DrawNodeChildren(ScenePtr.get());
        ImGui::TreePop();
    }    
}


void object3D::DrawMaterial(){}
void object3D::DrawCustomGUI(){}
void object3D::DrawGUI()
{
    if(ImGui::IsKeyPressed(ImGuiKey_Delete))
    {
        context::Get()->Scene->DeleteObject(context::Get()->Scene->SceneGUI->NodeClicked);
        context::Get()->Scene->SceneGUI->NodeClicked=nullptr;
        return;
    }

    if(context::Get()->CtrlPressed && ImGui::IsKeyPressed(ImGuiKey_D))
    {
        std::shared_ptr<object3D> Clone = this->Clone(false);
        transform::DoCompute=false;
        this->Parent->AddObject(Clone);
        transform::DoCompute=true;
    }

    ImGuiTabBarFlags TabBarFlags = ImGuiTabBarFlags_None;
    ImGui::Text(this->Name.c_str());
    if (ImGui::BeginTabBar("", TabBarFlags))
    {
        if(ImGui::BeginTabItem("Object"))
        {
            v3f LocalPosition = Transform.LocalValues.LocalPosition;
            v3f LocalRotation = Transform.LocalValues.LocalRotation;
            v3f LocalScale = Transform.LocalValues.LocalScale;
            if(ImGui::DragFloat3("Position", (float*)&LocalPosition, 0.01f))
            {
                this->Transform.SetLocalPosition(LocalPosition);
            }
            if(ImGui::DragFloat3("Rotation", (float*)&LocalRotation, 0.5f))
            {
                this->Transform.SetLocalRotation(LocalRotation);
            }
            if(ImGui::DragFloat3("Scale", (float*)&LocalScale, 0.01f))
            {
                this->Transform.SetLocalScale(LocalScale);
            }
            ImGui::EndTabItem();
        }
        
        DrawCustomGUI();

        ImGui::EndTabBar();
    }
}

void mesh::DrawCustomGUI()
{
    if (ImGui::BeginTabItem("Material"))
    {
        DrawMaterial();
        ImGui::EndTabItem();
    }
}

void mesh::DrawMaterial()
{
    if(ImGui::Button("Set material"))
    {
        ImGui::OpenPopup("Material Selection");
    }
    ShowMaterialSelection(this->Material);
    
    this->Material->DrawGUI();
}


void mesh::ShowMaterialSelection(std::shared_ptr<material> &Material)
{
    if(ImGui::BeginPopupModal("Material Selection"))
    {
        context::project &Project = context::Get()->Project;
        for (auto &Material : Project.Materials)
        {
            ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
            if(SelectedMaterial.get() == Material.get()) Flags |= ImGuiTreeNodeFlags_Selected;
            ImGui::TreeNodeEx(Material->Name.c_str(), Flags);
            if(ImGui::IsItemClicked())
            {
                SelectedMaterial = Material;
            }
            ImGui::TreePop();
        }
        
        if (ImGui::Button("Select"))
        {
            gfx::pipelineHandle OldPipeline = Material->PipelineHandle;
            Material = SelectedMaterial;
            context::Get()->Scene->UpdateMeshPipeline(OldPipeline, this);
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Button("Close"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
void light::DrawCustomGUI()
{
    if (ImGui::BeginTabItem("Light"))
    {
        b8 Changed = false;
        s32 LightType = (s32) this->Data.SizeAndType.w;
        Changed |= ImGui::Combo("Render Mode", &LightType, "Point\0Directional\0Spot\0Area\0Rasterizer\0PathTraceCompute\0\0");
        this->Data.SizeAndType.w = (f32)LightType;
        
        ImGui::Text("Shadow Parameters");

        ImGui::Checkbox("Automatic Frustum", &this->AutomaticShadowFrustum);
        if(this->AutomaticShadowFrustum)
        {
            ImGui::Checkbox("Use Camera Far Plane", &this->UseMainCameraFarPlane);
            if(!UseMainCameraFarPlane)
            {
                ImGui::DragFloat("Camera Far Plane Mult", &this->ShadowDistance, 0.1f, 0.1f);
            }
        }
        else
        {
            ImGui::DragFloat3("Frustum Size", &ShadowFrustumSize[0], 1, 1);
        }
        
        ImGui::Separator();

        Changed |= ImGui::ColorEdit3("Color", glm::value_ptr(this->Data.ColorAndIntensity));
        Changed |= ImGui::SliderFloat("Intensity", &this->Data.ColorAndIntensity.w, 0, 100);

            
        if(Changed)
        {
            context::Get()->Scene->UpdateLight(this->IndexInScene);
        }
        ImGui::EndTabItem();
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////


void pbrMaterial::DrawGUI()
{
    ImGui::Separator();
    ImGui::Text(this->Name.c_str());
    bool ShouldUpdate = false;
    bool ShouldRecreatePipeline = false;
    ShouldUpdate |= ImGui::ColorEdit3("Base Color", glm::value_ptr(this->UniformData.BaseColorFactor));
    ShouldUpdate |= ImGui::ColorEdit3("Emission", glm::value_ptr(this->UniformData.Emission));
    ShouldUpdate |= ImGui::DragFloat("Opacity", &this->UniformData.OpacityFactor, 0.005f, 0, 1);
    ShouldUpdate |= ImGui::DragFloat("Occlusion Strength", &this->UniformData.OcclusionStrength, 0.005f, 0, 1);
    ShouldUpdate |= ImGui::DragFloat("Emission Strength", &this->UniformData.EmissiveFactor, 0.005f, 0, 1);
    ShouldUpdate |= ImGui::DragFloat("Roughness", &this->UniformData.RoughnessFactor, 0.005f, 0, 1);
    ShouldUpdate |= ImGui::DragFloat("Metallic", &this->UniformData.MetallicFactor, 0.005f, 0, 1);
    ShouldUpdate |= ImGui::DragFloat("Alpha Cutoff", &this->UniformData.AlphaCutoff, 0.005f, 0, 1);
    

    if(DrawTexture("Base Color Texture", this->BaseColorTexture, this->UniformData.UseBaseColor))
    {
        SetBaseColorTexture(this->BaseColorTexture);
        ShouldUpdate=true;
    }

    if(DrawTexture("Occlusion Texture", this->OcclusionTexture, this->UniformData.UseOcclusionTexture))
    {
        SetOcclusionTexture(this->OcclusionTexture);
        ShouldUpdate=true;
    }

    if(DrawTexture("Emissive Texture", this->EmissiveTexture, this->UniformData.UseEmissionTexture))
    {
        SetEmissiveTexture(this->EmissiveTexture);
        ShouldUpdate=true;
    }

    if(DrawTexture("Normal Texture", this->NormalTexture, this->UniformData.UseNormalTexture))
    {
        SetNormalTexture(this->NormalTexture);
        ShouldUpdate=true;
    }

    if(DrawTexture("Metallic/Roughness Texture", this->MetallicRoughnessTexture, this->UniformData.UseMetallicRoughnessTexture))
    {
        SetMetallicRoughnessTexture(this->MetallicRoughnessTexture);
        ShouldUpdate=true;
    }

    bool DepthWriteEnabled = this->Flags & materialFlags::DepthWriteEnabled;
    if(ImGui::Checkbox("Depth Write", &DepthWriteEnabled))
    {
        Flags = (materialFlags::bits)(Flags ^ materialFlags::DepthWriteEnabled);
        this->ShouldRecreate = true;
    }

    bool DepthTestEnabled = this->Flags & materialFlags::DepthTestEnabled;
    if(ImGui::Checkbox("Depth Test", &DepthTestEnabled))
    {
        Flags = (materialFlags::bits)(Flags ^ materialFlags::DepthTestEnabled);
        this->ShouldRecreate = true;
    }

    bool DoubleSided = !(this->Flags & materialFlags::CullModeOn);
    if(ImGui::Checkbox("Double Sided", &DoubleSided))
    {
        Flags = (materialFlags::bits)(Flags ^ materialFlags::CullModeOn);
        this->ShouldRecreate = true;
    }

    bool BlendEnabled = this->Flags & materialFlags::BlendEnabled;
    if(ImGui::Checkbox("Transparent", &BlendEnabled))
    {
        Flags = (materialFlags::bits)(Flags ^ materialFlags::BlendEnabled);
        this->ShouldRecreate = true;
    }


    if(ShouldUpdate)
        Update();
}


b8 pbrMaterial::ShowTextureSelection(const char *ID, std::shared_ptr<texture> &Texture)
{
    const f32 ImageWidth = 50;
    const f32 ImageHeight = 30;
    // f32 Offset = ImageHeight / 2 - TextHeight/2;

    bool Changed=false;
    if(ImGui::BeginPopupModal(ID))
    {
        context::project &Project = context::Get()->Project;
        for (auto &Tex : Project.Textures)
        {
            ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_Leaf;
            if(SelectedTexture.get() == Tex.get()) Flags |= ImGuiTreeNodeFlags_Selected;
            
            gfx::image *Image = gfx::context::Get()->GetImage(Tex->Handle);
            ImGui::Image(Image->GetImGuiID(), ImVec2(ImageWidth, ImageHeight));
            ImGui::SameLine();
            ImGui::TreeNodeEx(Tex->Name.c_str(), Flags);
            if(ImGui::IsItemClicked())
            {
                SelectedTexture = Tex;
            }
            ImGui::TreePop();
        }
        
        if (ImGui::Button("Select"))
        {
            Texture = SelectedTexture;
            ImGui::CloseCurrentPopup();
            Changed = true;
        }
        if (ImGui::Button("Close"))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
    return Changed;
}

bool pbrMaterial::DrawTexture(const char *Name, std::shared_ptr<texture> &Texture, f32 &Use)
{   
    b8 Changed=false;
    f32 TextHeight = ImGui::CalcTextSize(Name).y;
    const f32 ImageWidth = 50;
    const f32 ImageHeight = 30;
    f32 Offset = ImageHeight / 2 - TextHeight/2;

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + Offset);
    b8 UseBool = Use > 0.0f; 
    if(ImGui::Checkbox(Name, &UseBool))
    {
        Use = UseBool ? 1.0f : 0.0f;
        Changed = true;
    }

    ImGui::SameLine();

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - Offset);
    gfx::image *Image = gfx::context::Get()->GetImage(Texture->Handle);
    ImGui::Image(Image->GetImGuiID(), ImVec2(ImageWidth, ImageHeight));
    if (ImGui::IsItemHovered() || ImGui::IsItemFocused())
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    if(ImGui::IsItemClicked())
    {
        ImGui::OpenPopup(Name);
    }
    Changed |= ShowTextureSelection(Name, Texture);            
 
 
    ImGui::SameLine();
    gfx::image *BlackImage = gfx::context::Get()->GetImage(defaultTextures::BlackTexture->Handle);
    ImGui::Image(BlackImage->GetImGuiID(), ImVec2(30, ImageHeight));
    if(ImGui::IsItemClicked())
    {
        Changed = true;
        Texture = defaultTextures::BlackTexture;
    }

    ImGui::SameLine();
    gfx::image *WhiteImage = gfx::context::Get()->GetImage(defaultTextures::WhiteTexture->Handle);
    ImGui::Image(WhiteImage->GetImGuiID(), ImVec2(30, ImageHeight));
    if(ImGui::IsItemClicked())
    {
        Changed = true;
        Texture = defaultTextures::WhiteTexture;
    }

    ImGui::SameLine();
    gfx::image *BlueImage = gfx::context::Get()->GetImage(defaultTextures::BlueTexture->Handle);
    ImGui::Image(BlueImage->GetImGuiID(), ImVec2(30, ImageHeight));
    if(ImGui::IsItemClicked())
    {
        Changed = true;
        Texture = defaultTextures::BlueTexture;
    }
    
    
    return Changed;
}



}