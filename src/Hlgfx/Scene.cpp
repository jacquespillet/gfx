#include "gfx/Include/Context.h"
#include "Include/Context.h"

#include "Include/Scene.h"
#include "Include/Mesh.h"
#include "Include/Material.h"
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
    
}

void scene::AddObject(std::shared_ptr<object3D> Object)
{
    object3D::AddObject(Object);
}

void scene::AddMesh(std::shared_ptr<object3D> Object)
{
    // If it's a mesh, store it in the meshes map as well
    std::shared_ptr<mesh> Mesh = std::dynamic_pointer_cast<mesh>(Object);
    if(Mesh)
    {
        this->Meshes[Mesh->Material->PipelineHandle].push_back(Mesh);
    }
}

void scene::OnRender(std::shared_ptr<camera> Camera)
{
    for(auto &PipelineMeshes : this->Meshes)
    {
        if (PipelineMeshes.second.size() == 0) continue;
        std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();
        CommandBuffer->BindGraphicsPipeline(PipelineMeshes.first);
        CommandBuffer->BindUniformGroup(Camera->Uniforms, CameraUniformsBinding);
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

        if(ImGui::IsItemClicked())
        {
            //Unselect previous
            if(NodeClicked != nullptr) NodeClicked->IsSelectedInGui=false;

            //Unselect if already clicked
            if(NodeClicked == Object->Children[i]) NodeClicked = nullptr;
            else //Otherwise select
            {
                NodeClicked = Object->Children[i];
                NodeClicked->IsSelectedInGui=true;
            }
        }

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
            DrawNodeChildren(Object->Children[i].get());
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
        NodeClicked->IsSelectedInGui=false;
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
}

void scene::DeleteObject(std::shared_ptr<object3D> Object)
{
    //Go through all children and delete them recursively
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
    ImGui::Begin("Scene", nullptr, WindowFlags);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(1,1,1,1));

    ImGui::BeginChild("Scene", ImVec2(GuiWidth, 300));
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

void scene::LoadFromFile(const char *FileName)
{
    transform::DoCompute=false;
    this->Clear();
    
    std::ifstream InStream;
    InStream.open(FileName, std::ios_base::binary);
    if(!InStream.is_open())
    {
        printf("Could not open file %s", FileName);
        assert(false);
    }

    u32 NumChildren;
    InStream.read((char*)&NumChildren, sizeof(u32));

    for(u32 i=0; i<NumChildren; i++)
    {
        u32 ChildSize;
        InStream.read((char*)&ChildSize, sizeof(u32));
        std::vector<u8> SerializedChild(ChildSize);
        InStream.read((char*)SerializedChild.data(), ChildSize);
        std::shared_ptr<object3D> Object3D = object3D::Deserialize(SerializedChild);
        this->AddObject(Object3D);
    }

    InStream.close();
    
    transform::DoCompute=true;
}

void scene::SaveToFile(const char *FileName)
{
    std::ofstream OutStream;
    OutStream.open(FileName, std::ios_base::binary | std::ios_base::trunc);
    if(!OutStream.is_open())
    {
        printf("Could not open file %s", FileName); 
        assert(false);
    }

    u32 NumChildren = this->Children.size();
    OutStream.write((char*)&NumChildren, sizeof(u32));
    for(u32 i=0; i<NumChildren; i++)
    {
        std::vector<u8> SerializedChild = this->Children[i]->Serialize();
        u32 ChildSize = SerializedChild.size();
        OutStream.write((char*)&ChildSize, sizeof(u32));
        OutStream.write((char*)SerializedChild.data(), SerializedChild.size());
    }

    OutStream.close();
}

}