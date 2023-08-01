#include "Include/Object3D.h"
#include "Include/Camera.h"
#include "Include/Context.h"
#include "Include/Util.h"
#include "Include/Scene.h"
#include "Include/Mesh.h"
#include "Include/Material.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/ext.hpp>

#include <stb_image_write.h>

namespace hlgfx
{
object3D::object3D(const char *Name)
{
    this->Name = Name;
    this->Parent = nullptr;
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
    return nullptr;
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
    if(Object->Parent != nullptr)
    {
        Object->DeleteChild(Object);
    }
    context::Get()->Scene->AddMesh(Object);

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
        m4x4 ModelMatrix = this->Transform.Matrices.LocalToWorld;
    
        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        ImGuizmo::Manipulate(glm::value_ptr(Camera->Data.ViewMatrix), glm::value_ptr(Camera->Data.ProjectionMatrix), context::Get()->CurrentGizmoOperation, context::Get()->CurrentGizmoMode, glm::value_ptr(ModelMatrix), NULL, NULL);

        //Remove the localToWorld component
        ModelMatrix = glm::inverse(this->Transform.Parent->Matrices.LocalToWorld) * ModelMatrix;

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

void object3D::DrawMaterial()
{

}

void object3D::DrawGUI()
{
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
            DrawMaterial();
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }
}


std::vector<u8> object3D::Serialize()
{
    std::vector<u8> Result;
    
    u32 Object3DType = (u32) object3DType::Object3d;
    AddItem(Result, &Object3DType, sizeof(u32));

    u32 StringLength = this->Name.size();
    AddItem(Result, &StringLength, sizeof(u32));
    AddItem(Result, (void*)this->Name.data(), StringLength);
    
    AddItem(Result, &this->Transform.Matrices, sizeof(transform::matrices));
    AddItem(Result, &this->Transform.LocalValues, sizeof(transform::localValues));

    u32 NumChildren = this->Children.size();
    AddItem(Result, &NumChildren, sizeof(u32));

    for (sz i = 0; i < NumChildren; i++)
    {
        std::vector<u8> ChildBlob = this->Children[i]->Serialize();
        u32 ChildrenSize = ChildBlob.size();
        AddItem(Result, &ChildrenSize, sizeof(u32));
        AddItem(Result, ChildBlob.data(), ChildBlob.size());
    }
    
    return Result;
}

void object3D::DeleteChild(std::shared_ptr<object3D> Child)
{
    u32 Index = 0;
    if(FindInChildren(this->Children, Child.get(), Index))
    {
        this->Children.erase(this->Children.begin() + Index);
    }
}

void GetItem(std::vector<u8> &Blob, void *Dest, u32 &Cursor, sz Size)
{
    memcpy(Dest, Blob.data() + Cursor, Size);
    Cursor += Size;
}


std::shared_ptr<object3D> object3D::Deserialize(std::vector<u8> &Serialized)
{
    u32 Cursor = 0;
    
    u32 ObjectType;
    GetItem(Serialized, &ObjectType, Cursor, sizeof(u32));

    u32 NameLength;
    GetItem(Serialized, &NameLength, Cursor, sizeof(u32));

    //TODO: Refactor that
    if(ObjectType == (u32)object3DType::Object3d)
    {
        std::shared_ptr<object3D> Result = std::make_shared<object3D>("BLA");
        
        Result->Name.resize(NameLength);
        GetItem(Serialized, (void*)Result->Name.data(), Cursor, NameLength);


        GetItem(Serialized, &Result->Transform.Matrices, Cursor, sizeof(transform::matrices));
        GetItem(Serialized, &Result->Transform.LocalValues, Cursor, sizeof(transform::localValues));
        Result->Transform.HasChanged=true;

        u32 NumChildren;
        GetItem(Serialized, &NumChildren, Cursor, sizeof(u32));

        for (sz i = 0; i < NumChildren; i++)
        {
            u32 ChildSize;
            GetItem(Serialized, &ChildSize, Cursor, sizeof(u32));
            std::vector<u8> SerializedChild(ChildSize);
            GetItem(Serialized, SerializedChild.data(), Cursor, ChildSize);

            std::shared_ptr<object3D> Object = object3D::Deserialize(SerializedChild);
            Result->AddObject(Object);
        }
        return Result;  
    }
    else if(ObjectType == (u32) object3DType::Mesh)
    {
        std::shared_ptr<mesh> Result = std::make_shared<mesh>();

        Result->Name.resize(NameLength);
        GetItem(Serialized, (void*)Result->Name.data(), Cursor, NameLength);

        sz VertexDataSize;
        GetItem(Serialized, &VertexDataSize, Cursor, sizeof(sz));
        std::vector<u8> VertexData(VertexDataSize);
        GetItem(Serialized, VertexData.data(), Cursor, VertexDataSize);
        
        sz IndexDataSize;
        GetItem(Serialized, &IndexDataSize, Cursor, sizeof(sz));
        std::vector<u8> IndexData(IndexDataSize);
        GetItem(Serialized, IndexData.data(), Cursor, IndexDataSize);

        Result->GeometryBuffers = CreateGeometryFromBuffers(VertexData.data(), VertexData.size(), IndexData.data(), IndexData.size());

        u32 MaterialType;
        GetItem(Serialized, &MaterialType, Cursor, sizeof(u32));
        if(MaterialType == (u32)materialType::Unlit)
        {
            materialFlags::bits Flags;
            GetItem(Serialized, &Flags, Cursor, sizeof(u32));
            std::shared_ptr<unlitMaterial> Material = std::make_shared<unlitMaterial>(Flags);
            GetItem(Serialized, glm::value_ptr(Material->UniformData.BaseColorFactor), Cursor, sizeof(v4f));

            u8 HasDiffuseTexture = 0;
            GetItem(Serialized, &HasDiffuseTexture, Cursor, sizeof(u8));
            if(HasDiffuseTexture)
            {
                gfx::imageData ImageData = {};
                GetItem(Serialized, &ImageData.Width, Cursor, sizeof(u32));
                GetItem(Serialized, &ImageData.Height, Cursor, sizeof(u32));
                GetItem(Serialized, &ImageData.Format, Cursor, sizeof(u32));
                GetItem(Serialized, &ImageData.ChannelCount, Cursor, sizeof(u8));
                GetItem(Serialized, &ImageData.Type, Cursor, sizeof(gfx::type));
                GetItem(Serialized, &ImageData.DataSize, Cursor, sizeof(sz));
                ImageData.Data = (u8*) gfx::AllocateMemory(ImageData.DataSize);
                GetItem(Serialized, ImageData.Data, Cursor, ImageData.DataSize);

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

                gfx::imageHandle ImageHandle = gfx::context::Get()->CreateImage(ImageData, ImageCreateInfo);
                Material->SetBaseColorTexture(std::make_shared<texture>(ImageHandle));

                gfx::DeallocateMemory(ImageData.Data);
            }

            Result->Material = Material;
        }
     
        GetItem(Serialized, &Result->Transform.Matrices, Cursor, sizeof(transform::matrices));
        GetItem(Serialized, &Result->Transform.LocalValues, Cursor, sizeof(transform::localValues));
        Result->Transform.HasChanged=true;

        u32 NumChildren;
        GetItem(Serialized, &NumChildren, Cursor, sizeof(u32));

        for (sz i = 0; i < NumChildren; i++)
        {
            u32 ChildSize;
            GetItem(Serialized, &ChildSize, Cursor, sizeof(u32));
            std::vector<u8> SerializedChild(ChildSize);
            GetItem(Serialized, SerializedChild.data(), Cursor, ChildSize);

            std::shared_ptr<object3D> Object = object3D::Deserialize(SerializedChild);
            Result->AddObject(Object);
        }
        return Result; 

    }
}

object3D::~object3D()
{
}

}