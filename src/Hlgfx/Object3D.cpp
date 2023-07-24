#include "Include/Object3D.h"

namespace hlgfx
{
object3D::object3D(object3D *Parent)
{
    this->Parent = Parent;
    if(this->Parent != nullptr)
        this->Transform.Parent = &Parent->Transform;
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


void object3D::AddObject(std::shared_ptr<object3D> Object)
{
    this->Children.push_back(Object);
    this->Transform.Children.push_back(&Object->Transform);
}

void object3D::OnBeforeRender()
{
    for (sz i = 0; i < Children.size(); i++)
    {
        Children[i]->OnBeforeRender();
    }
}

void object3D::OnRender()
{
    for (sz i = 0; i < Children.size(); i++)
    {
        Children[i]->OnRender();
    }
}

void object3D::OnUpdate()
{
    for (sz i = 0; i < Children.size(); i++)
    {
        Children[i]->OnUpdate();
    }
}

void object3D::OnAfterRender()
{
    for (sz i = 0; i < Children.size(); i++)
    {
        Children[i]->OnAfterRender();
    }
}

}