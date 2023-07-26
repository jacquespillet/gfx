#include "Include/Object3D.h"
#include "Include/Camera.h"
#include "Include/Context.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/ext.hpp>

namespace hlgfx
{
object3D::object3D(const char *Name)
{
    this->Name = Name;
}


void object3D::SetRenderOrder(u32 RenderOrder)
{
    this->RenderOrder = RenderOrder;
}

void object3D::SetFrustumCulled(b8 FrustumCulled)
{
    this->FrustumCulled = FrustumCulled;
}

void object3D::SetCastShadow(b8 CastShadow)
{
    this->CastShadow = CastShadow;
}

void object3D::SetReceiveShadow(b8 ReceiveShadow)
{
    this->ReceiveShadow = ReceiveShadow;
}

void RemoveObjectFromChildren(std::vector<std::shared_ptr<object3D>> &Children, object3D *Object)
{
    int Index=-1;
    for (size_t i = 0; i < Children.size(); i++)
    {
        if(Children[i].get() == Object)
        {
            Index = i;
            break;
        }
    }
    if(Index != -1)
    {
        Children.erase(Children.begin() + Index);
    }
}

std::shared_ptr<object3D> FindInChildren(std::vector<std::shared_ptr<object3D>> &Children, object3D *Object, u32 &OutIndex)
{
    for (size_t i = 0; i < Children.size(); i++)
    {
        if(Children[i].get() == Object)
        {
            OutIndex = i;
            return Children[i];
        }
    }
    
}

void object3D::SetParent(std::shared_ptr<object3D> Parent)
{
    object3D *OldParent = this->Parent;
    assert(OldParent != nullptr);

    this->Parent = Parent.get();
    this->Transform.SetParent(&Parent->Transform);

    u32 Index = 0;
    std::shared_ptr<object3D> ThisReference = FindInChildren(OldParent->Children, this, Index);

    this->Parent->Children.push_back(ThisReference);
    this->Parent->Transform.Children.push_back(&this->Transform);

    OldParent->Children.erase(OldParent->Children.begin() + Index);
    auto End = std::remove(OldParent->Transform.Children.begin(), OldParent->Transform.Children.end(), &this->Transform);
    OldParent->Transform.Children.erase(End, OldParent->Transform.Children.end());
}

void object3D::AddObject(std::shared_ptr<object3D> Object)
{
    //TODO: Case where object already has a parent (needs to be removed)
    
    Object->Parent = this;
    Object->Transform.SetParent(&this->Transform);
    
    this->Children.push_back(Object);
    this->Transform.Children.push_back(&Object->Transform);
}

void object3D::OnEarlyUpdate()
{
    for (sz i = 0; i < Children.size(); i++)
    {
        Children[i]->Transform.HasChanged=false;
        Children[i]->OnEarlyUpdate();
    }
}


void object3D::OnBeforeRender(std::shared_ptr<camera> Camera)
{
    for (sz i = 0; i < Children.size(); i++)
    {
        Children[i]->OnBeforeRender(Camera);
    }
    
    if(IsSelectedInGui)
    {
        m4x4 ModelMatrix = this->Transform.LocalToWorld;
    
        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        ImGuizmo::Manipulate(glm::value_ptr(Camera->Data.ViewMatrix), glm::value_ptr(Camera->Data.ProjectionMatrix), context::Get()->CurrentGizmoOperation, context::Get()->CurrentGizmoMode, glm::value_ptr(ModelMatrix), NULL, NULL);

        //Remove the localToWorld component
        ModelMatrix = glm::inverse(this->Transform.Parent->LocalToWorld) * ModelMatrix;

        //Decompose the matrix
        v3f matrixTranslation, matrixRotation, matrixScale;
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(ModelMatrix), glm::value_ptr(matrixTranslation), glm::value_ptr(matrixRotation), glm::value_ptr(matrixScale));
        //Set local properties
        this->Transform.SetLocalPosition(matrixTranslation);
        this->Transform.SetLocalRotation(matrixRotation);
        this->Transform.SetLocalScale(matrixScale);
    }
}

void object3D::OnRender(std::shared_ptr<camera> Camera)
{
    for (sz i = 0; i < Children.size(); i++)
    {
        Children[i]->OnRender(Camera);
    }

    ImGuiIO& io = ImGui::GetIO();
    
}

void object3D::OnUpdate()
{
    for (sz i = 0; i < Children.size(); i++)
    {
        Children[i]->OnUpdate();
    }
}


void object3D::OnAfterRender(std::shared_ptr<camera> Camera)
{
    for (sz i = 0; i < Children.size(); i++)
    {
        Children[i]->OnAfterRender(Camera);
    }
}

void object3D::DrawGUI()
{
    ImGuiTabBarFlags TabBarFlags = ImGuiTabBarFlags_None;
    ImGui::Text(this->Name);
    if (ImGui::BeginTabBar("", TabBarFlags))
    {
        if(ImGui::BeginTabItem("Object"))
        {
            v3f LocalPosition = Transform.LocalPosition;
            v3f LocalRotation = Transform.LocalRotation;
            v3f LocalScale = Transform.LocalScale;
            if(ImGui::DragFloat3("Position", (float*)&LocalPosition, 0.01f))
            {
                this->Transform.SetLocalPosition(LocalPosition);
            }
            if(ImGui::DragFloat3("Rotation", (float*)&LocalRotation, 0.01f))
            {
                this->Transform.SetLocalRotation(LocalRotation);
            }
            if(ImGui::DragFloat3("Scale", (float*)&LocalScale, 0.01f))
            {
                this->Transform.SetLocalScale(LocalScale);
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Geometry"))
        {
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Material"))
        {
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}

}