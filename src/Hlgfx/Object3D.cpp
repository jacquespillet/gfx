#include "Include/Object3D.h"

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
}

void object3D::OnRender(std::shared_ptr<camera> Camera)
{
    for (sz i = 0; i < Children.size(); i++)
    {
        Children[i]->OnRender(Camera);
    }
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

}