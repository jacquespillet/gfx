#include "gfx/Include/Context.h"
#include "Include/Context.h"

#include "Include/Scene.h"
#include "Include/Mesh.h"
#include "Include/Material.h"
#include <imgui.h>

namespace hlgfx
{
scene::scene() : object3D("Scene")
{
    
}

void scene::AddObject(std::shared_ptr<object3D> Object)
{
    object3D::AddObject(Object);

    //If it's a mesh, store it in the meshes map as well
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
        if(NumChildren == 0) NodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        if(Object->Children[i] == NodeClicked) NodeFlags |= ImGuiTreeNodeFlags_Selected;

        bool NodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, NodeFlags, Object->Children[i]->Name, i);
        if(NumChildren == 0) NodeOpen=false;


        if(ImGui::IsItemClicked())
        {
            if(NodeClicked != nullptr) NodeClicked->IsSelectedInGui=false;
            NodeClicked = Object->Children[i];
            NodeClicked->IsSelectedInGui=true;
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
    if(NumChildren == 0) BaseFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;

    bool NodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)0, BaseFlags, this->Name);
    if(NumChildren == 0) NodeOpen=false;

    if(ImGui::IsItemClicked())
    {
        NodeClicked = ScenePtr;
        NodeClicked->IsSelectedInGui=false;
    }

    //Drag and drop
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
    ImGui::Text("Scene");
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

}