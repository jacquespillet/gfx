#include "gfx/Include/Context.h"
#include "Include/Context.h"

#include "Include/Scene.h"
#include "Include/Mesh.h"
#include "Include/Material.h"
#include "Include/Bindings.h"
#include <imgui.h>

#include <fstream>

namespace hlgfx
{

void RemoveObjectInChildren(std::vector<std::shared_ptr<object3D>>& Children, std::shared_ptr<object3D> Object) {
    auto it = std::find(Children.begin(), Children.end(), Object);
    if (it != Children.end()) {
        Children.erase(it);
    }
}
void RemoveMeshInChildren(std::vector<std::shared_ptr<mesh>>& Children, std::shared_ptr<mesh> Object) {
    auto it = std::find(Children.begin(), Children.end(), Object);
    if (it != Children.end()) {
        Children.erase(it);
    }
}

scene::scene() : object3D("Scene")
{
    this->UUID = context::Get()->GetUUID();
}

void scene::UpdateMeshPipeline(gfx::pipelineHandle OldPipelineHandle, mesh *Mesh)
{
    std::vector<std::shared_ptr<mesh>> &Vec = this->Meshes[OldPipelineHandle];
    sz IndexToRemove = (sz)-1;
    std::shared_ptr<mesh> MeshPtr = nullptr;
    for (sz i = 0; i < Vec.size(); i++)
    {
        if(Vec[i].get() == Mesh) 
        {
            MeshPtr = Vec[i];
            IndexToRemove = i; 
            break;
        }
    }

    assert(IndexToRemove != (sz)-1);
    assert(MeshPtr);
    
    Vec.erase(Vec.begin() + IndexToRemove);

    gfx::pipelineHandle NewPipeline = Mesh->Material->PipelineHandle;
    if(this->Meshes.find(NewPipeline) == this->Meshes.end()) this->Meshes[NewPipeline] = {};
    this->Meshes[NewPipeline].push_back(MeshPtr);
}

void scene::AddObject(std::shared_ptr<object3D> Object)
{
    object3D::AddObject(Object);
    this->AddMeshesInObject(Object);
    this->NodeClicked = Object;
}

void scene::AddMeshesInObject(std::shared_ptr<object3D> Object)
{
    // If it's a mesh, store it in the meshes map as well
    std::shared_ptr<mesh> Mesh = std::dynamic_pointer_cast<mesh>(Object);
    if(Mesh)
    {
        this->Meshes[Mesh->Material->PipelineHandle].push_back(Mesh);
    }

    for (sz i = 0; i < Object->Children.size(); i++)
    {
        AddMeshesInObject(Object->Children[i]);
    }
    
}

void scene::OnRender(std::shared_ptr<camera> Camera)
{
    for(auto &PipelineMeshes : this->Meshes)
    {
        if (PipelineMeshes.second.size() == 0) continue;
        std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();
        CommandBuffer->BindGraphicsPipeline(PipelineMeshes.first);
        CommandBuffer->BindUniformGroup(Camera->Uniforms, CameraDescriptorSetBinding);
        for(sz i=0; i<PipelineMeshes.second.size(); i++)
        {
            PipelineMeshes.second[i]->OnRender(Camera);
        }
    }
}



void scene::DrawNodeChildren(hlgfx::object3D *Object)
{
    //For each children, draw it
    static ImGuiTreeNodeFlags BaseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    for(size_t i=0; i<Object->Children.size(); i++)
    {
        ImGuiTreeNodeFlags NodeFlags = BaseFlags;
        
        int NumChildren = Object->Children[i]->Children.size();
        if(NumChildren == 0) NodeFlags |= ImGuiTreeNodeFlags_Leaf;
        if(Object->Children[i] == NodeClicked) NodeFlags |= ImGuiTreeNodeFlags_Selected;

        bool NodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, NodeFlags, Object->Children[i]->Name.c_str(), i);
        
        if(ImGui::IsItemClicked(0) || ImGui::IsItemClicked(1))
        {
            if(NodeClicked == Object->Children[i] && ImGui::IsItemClicked(0)) NodeClicked = nullptr;
            else NodeClicked = Object->Children[i];
        }

        ImGui::PushID(i);
        if (ImGui::BeginPopupContextItem("Actions"))
        {
            context::Get()->DrawObjectMenu();
            if(ImGui::BeginMenu("Add"))
            {
                context::Get()->AddObjectMenu();
                ImGui::EndMenu();
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

void scene::DrawSceneGUI()
{
    std::shared_ptr<scene> ScenePtr = context::Get()->Scene;

    static ImGuiTreeNodeFlags BaseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
    int NumChildren = this->Children.size();
    if(NumChildren == 0) BaseFlags |= ImGuiTreeNodeFlags_Leaf;

    bool NodeOpen = ImGui::TreeNodeEx("Scene", BaseFlags, this->Name.c_str());

    if(ImGui::IsItemClicked())
    {
        NodeClicked = ScenePtr;
    }

    // Drag and drop
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        ImGui::SetDragDropPayload("DND_DEMO_CELL", &ScenePtr, sizeof(std::shared_ptr<hlgfx::object3D>));    // Set payload to carry the index of our item (could be anything)
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

void ClearObject(std::shared_ptr<object3D> Object)
{
    //If it's a mesh, we have to remove it from the meshes array
    std::shared_ptr<mesh> Mesh = std::dynamic_pointer_cast<mesh>(Object);
    if(Mesh)
    {
        RemoveMeshInChildren(context::Get()->Scene->Meshes[Mesh->Material->PipelineHandle], Mesh);
    }

    //Delete all children
    for (sz i = 0; i < Object->Children.size(); i++)
    {
        ClearObject(Object->Children[i]);
    }
    Object->Children.clear();
}

void scene::Clear()
{
    for (sz i = 0; i < Children.size(); i++)
    {
        ClearObject(Children[i]);
    }
    this->Children.clear();
    this->Meshes.clear();
    this->NodeClicked=nullptr;
}

void scene::DeleteObject(std::shared_ptr<object3D> Object)
{
    //Go through all children and delete them recursively
    if(NodeClicked == Object) NodeClicked = nullptr;
    ClearObject(Object);
    Object->Parent->DeleteChild(Object);
}

void scene::DrawGUI()
{
    ImGui::ShowDemoWindow();

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
            context::Get()->AddObjectMenu();
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

void scene::Serialize(std::string FilePath)
{
    std::ofstream FileStream;
    FileStream.open(FilePath, std::ios::trunc | std::ios::binary);
    assert(FileStream.is_open());
    Serialize(FileStream);  
}

void scene::Serialize(std::ofstream &FileStream)
{
    std::vector<u8> Result;

    u32 Object3DType = (u32) object3DType::Scene;
    FileStream.write((char*)&Object3DType, sizeof(u32));

    u32 UUIDSize = this->UUID.size();
    FileStream.write((char*)&UUIDSize, sizeof(u32));
    FileStream.write(this->UUID.data(), this->UUID.size());
    
    u32 StringLength = this->Name.size();
    FileStream.write((char*)&StringLength, sizeof(u32));
    FileStream.write((char*)(void*)this->Name.data(), StringLength);
    
    FileStream.write((char*)&this->Transform.Matrices, sizeof(transform::matrices));
    FileStream.write((char*)&this->Transform.LocalValues, sizeof(transform::localValues));
    

    u32 NumChildren = this->Children.size();
    FileStream.write((char*)&NumChildren, sizeof(u32));

    for (sz i = 0; i < NumChildren; i++)
    {
        this->Children[i]->Serialize(FileStream);
    }
}


}