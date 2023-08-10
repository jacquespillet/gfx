#include "gfx/Include/Context.h"
#include "Include/Context.h"

#include "Include/Scene.h"
#include "Include/GUI.h"
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

scene::scene(std::string Name) : object3D(Name)
{
    this->UUID = context::Get()->GetUUID();
    this->SceneGUI = std::make_shared<sceneGUI>(this);
}

std::shared_ptr<object3D> scene::Clone(b8 CloneUUID)
{
    std::shared_ptr<scene> Result = std::make_shared<scene>(this->Name);

    Result->RenderOrder=this->RenderOrder;
    Result->FrustumCulled=this->FrustumCulled;
    Result->CastShadow=this->CastShadow;
    Result->ReceiveShadow=this->ReceiveShadow;
    Result->Transform =  this->Transform;
    if(CloneUUID) Result->UUID = this->UUID;

    transform::DoCompute = false;
    for (sz i = 0; i < this->Children.size(); i++)
    {
        Result->AddObject(this->Children[i]->Clone(CloneUUID));
    }
    transform::DoCompute = true;
      
    return Result;    
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
    if(object3D::AddToScene) this->AddMeshesInObject(Object);
    this->SceneGUI->NodeClicked = Object;
}

void scene::AddMesh(std::shared_ptr<object3D> Object)
{
    std::shared_ptr<mesh> Mesh = std::dynamic_pointer_cast<mesh>(Object);
    if(Mesh)
    {
        std::vector<std::shared_ptr<mesh>> &Meshes = this->Meshes[Mesh->Material->PipelineHandle]; 
        if(std::find(Meshes.begin(), Meshes.end(), Mesh) == Meshes.end())
        {
            this->Meshes[Mesh->Material->PipelineHandle].push_back(Mesh);
        }
    }
}

void scene::AddMeshesInObject(std::shared_ptr<object3D> Object)
{
    // If it's a mesh, store it in the meshes map
    AddMesh(Object);

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

void scene::DrawGUI()
{
    this->SceneGUI->DrawGUI();
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
    this->SceneGUI->NodeClicked=nullptr;
}

void scene::DeleteObject(std::shared_ptr<object3D> Object)
{
    //Go through all children and delete them recursively
    if(this->SceneGUI->NodeClicked == Object) this->SceneGUI->NodeClicked = nullptr;
    ClearObject(Object);
    Object->Parent->DeleteChild(Object);
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