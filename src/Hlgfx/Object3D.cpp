#include "Include/Object3D.h"
#include "Include/Camera.h"
#include "Include/Context.h"
#include "Include/Util.h"
#include "Include/Scene.h"
#include "Include/Mesh.h"
#include "Include/Material.h"
#include "Include/GUI.h"
#include "Include/Bindings.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <glm/ext.hpp>


#include <stb_image_write.h>
#include <fstream>

namespace hlgfx
{

bool object3D::AddToScene = true;

object3D::object3D(std::string Name)
{
    this->Name = Name;
    this->Parent = nullptr;
    this->UUID = context::Get()->GetUUID();
}

std::shared_ptr<object3D> object3D::Clone(b8 CloneUUID)
{
    std::shared_ptr<object3D> Result = std::make_shared<object3D>(this->Name);

    Result->RenderOrder=this->RenderOrder;
    Result->FrustumCulled=this->FrustumCulled;
    Result->CastShadow=this->CastShadow;
    Result->ReceiveShadow=this->ReceiveShadow;
    if(CloneUUID) Result->UUID= this->UUID;
    Result->Transform =  this->Transform;
    Result->Transform.HasChanged=true;
    
    b8 DoCompute = transform::DoCompute;
    transform::DoCompute=false;
    for (sz i = 0; i < this->Children.size(); i++)
    {
        Result->AddObject(this->Children[i]->Clone(CloneUUID));
    }
    if(DoCompute) transform::DoCompute=true;
    
      
    return Result;
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
    Object->Parent = this;

    //When we're adding an object to a prefab, we don't want to add to the scene.
    if(AddToScene) context::Get()->Scene->AddMeshesInObject(Object);
    if(AddToScene) context::Get()->Scene->AddLightsInObject(Object);
    
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
    
    if(context::Get()->Scene->SceneGUI->NodeClicked.get() == this && context::Get()->Scene.get() != this)
    {
        m4x4 ModelMatrix = this->Transform.Matrices.LocalToWorld;
    
        ImGuiIO& io = ImGui::GetIO();
        ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
        if(ImGuizmo::Manipulate(glm::value_ptr(Camera->Data.ViewMatrix), glm::value_ptr(Camera->Data.ProjectionMatrix), context::Get()->GUI->CurrentGizmoOperation, context::Get()->GUI->CurrentGizmoMode, glm::value_ptr(ModelMatrix), NULL, NULL))
        {
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


void object3D::Serialize(std::string FilePath)
{
    std::ofstream FileStream;
    FileStream.open(FilePath, std::ios::trunc | std::ios::binary);
    assert(FileStream.is_open());
    Serialize(FileStream);  
}

void object3D::Serialize(std::ofstream &FileStream)
{
    u32 Object3DType = (u32) object3DType::Object3d;
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

std::shared_ptr<object3D> object3D::Deserialize(const std::string &FileName)
{
    transform::DoCompute = false;
    std::ifstream FileStream;
    FileStream.open(FileName, std::ios::binary);
    assert(FileStream.is_open());
    std::shared_ptr<object3D> Result = Deserialize(FileStream);      
    transform::DoCompute = true;
    return Result;
}

std::shared_ptr<object3D> object3D::Deserialize(std::ifstream &FileStream)
{
    u32 Object3DType;
    FileStream.read((char*)&Object3DType, sizeof(u32));

    u32 UUIDSize;
    FileStream.read((char*)&UUIDSize, sizeof(u32));
    std::string UUID; UUID.resize(UUIDSize);
    FileStream.read(UUID.data(), UUID.size());

    u32 NameLength;
    FileStream.read((char*)&NameLength, sizeof(u32));
    std::string Name; Name.resize(NameLength);
    FileStream.read((char*)Name.data(), Name.size());

    std::shared_ptr<object3D> Result;
    if(Object3DType == (u32)(object3DType::Mesh))
        Result = std::make_shared<mesh>(Name);
    else if(Object3DType == (u32)(object3DType::Scene))
        Result = std::make_shared<scene>(Name);
    else if(Object3DType == (u32)(object3DType::Object3d))
        Result = std::make_shared<object3D>(Name);
    else if(Object3DType == (u32)(object3DType::Light))
        Result = std::make_shared<light>(Name);

    Result->UUID = UUID;


    FileStream.read((char*)&Result->Transform.Matrices, sizeof(transform::matrices));
    FileStream.read((char*)&Result->Transform.LocalValues, sizeof(transform::localValues));

    if(Object3DType == (u32)(object3DType::Mesh))
    {
        std::shared_ptr<mesh> Mesh = std::static_pointer_cast<mesh>(Result);

        u32 GeometryUUIDSize;
        FileStream.read((char*)&GeometryUUIDSize, sizeof(u32));
        std::string GeometryUUID; GeometryUUID.resize(GeometryUUIDSize);
        FileStream.read(GeometryUUID.data(), GeometryUUIDSize);
        
        if(GeometryUUID == "DEFAULT_QUAD")
            Mesh->GeometryBuffers = context::Get()->Quad;
        else if(GeometryUUID == "DEFAULT_CUBE")
            Mesh->GeometryBuffers = context::Get()->Cube;
        else if(GeometryUUID == "DEFAULT_SPHERE")
            Mesh->GeometryBuffers = context::Get()->Sphere;
        else if(GeometryUUID == "DEFAULT_CONE")
            Mesh->GeometryBuffers = context::Get()->Cone;
        else if(GeometryUUID == "DEFAULT_CAPSULE")
            Mesh->GeometryBuffers = context::Get()->Capsule;
        else if(GeometryUUID == "DEFAULT_CYLINDER")
            Mesh->GeometryBuffers = context::Get()->Cylinder;
        else
            Mesh->GeometryBuffers = context::Get()->Project.Geometries[GeometryUUID];

        u32 MaterialUUIDSize;
        FileStream.read((char*)&MaterialUUIDSize, sizeof(u32));
        std::string MaterialUUID; MaterialUUID.resize(MaterialUUIDSize);
        FileStream.read(MaterialUUID.data(), MaterialUUIDSize);
        if(MaterialUUID == "NO_MATERIAL") Mesh->Material = context::Get()->NoMaterial;
        else Mesh->Material = context::Get()->Project.Materials[MaterialUUID];

        Mesh->UniformData.ModelMatrix = Mesh->Transform.Matrices.LocalToWorld;
        gfx::context::Get()->CopyDataToBuffer(Mesh->UniformBuffer, &Mesh->UniformData, sizeof(mesh::uniformData), 0);
        Mesh->Uniforms->Update();
    }
    else if(Object3DType == (u32)(object3DType::Light))
    {
        std::shared_ptr<light> Light = std::static_pointer_cast<light>(Result);
        FileStream.read((char*)&Light->Data, sizeof(light::lightData));
    }

    u32 NumChildren;
    FileStream.read((char*)&NumChildren, sizeof(u32));
    for (sz i = 0; i < NumChildren; i++)
    {
        std::shared_ptr<object3D> Child = Deserialize(FileStream);
        Result->AddObject(Child);
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



object3D::~object3D()
{
}

}