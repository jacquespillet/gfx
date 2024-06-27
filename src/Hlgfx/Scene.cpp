#include "gfx/Include/Context.h"
#include "Include/Context.h"

#include "Include/Scene.h"
#include "Include/GUI.h"
#include "Include/Mesh.h"
#include "Include/Renderer.h"
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

void ClearObject(std::shared_ptr<object3D> Object)
{
    //If it's a mesh, we have to remove it from the meshes array
    std::shared_ptr<mesh> Mesh = std::dynamic_pointer_cast<mesh>(Object);
    if(Mesh)
    {
        gfx::pipelineHandle MeshPipeline = context::Get()->Project.Materials[Mesh->MaterialID]->PipelineHandle;
        RemoveMeshInChildren(context::Get()->Scene->Meshes[MeshPipeline], Mesh);
    }

    std::shared_ptr<light> Light = std::dynamic_pointer_cast<light>(Object);
    if(Light)
    {
        std::shared_ptr<scene> Scene = context::Get()->Scene;
        std::shared_ptr<light> *Lights = Scene->Lights; 
        auto it = std::find(Lights, Lights + MaxLights, Light);
        sz Inx = std::distance(Lights, it);
        if (it != Lights + MaxLights) {
            memcpy(&Scene->SceneBufferData.Lights[Inx], &Scene->SceneBufferData.Lights[Inx+1], (Scene->SceneBufferData.LightCount - Inx-1) * sizeof(light::lightData));
            memcpy(&Scene->Lights[Inx], &Scene->Lights[Inx+1], (Scene->SceneBufferData.LightCount - Inx-1) * sizeof(std::shared_ptr<light>));
        }        
        Scene->SceneBufferData.LightCount--;
        Scene->UpdateLightsAfter(Inx);
    }

    //Delete all children
    for (sz i = 0; i < Object->Children.size(); i++)
    {
        ClearObject(Object->Children[i]);
    }
    Object->Children.clear();
}

scene::scene(std::string Name) : object3D(Name)
{
    this->ID = context::Get()->Project.Scenes.size();

    this->SceneGUI = std::make_shared<sceneGUI>(this);
    this->SceneBufferData.LightCount = 0;
    this->SceneBuffer = gfx::context::Get()->CreateBuffer(sizeof(sceneBuffer), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu, sizeof(sceneBuffer));
    
    this->Uniforms = std::make_shared<gfx::uniformGroup>();
    this->Uniforms->Reset();
    this->Uniforms->AddUniformBuffer(SceneBinding, this->SceneBuffer);
    this->Uniforms->AddTexture(ShadowMapsBinding, context::Get()->ShadowMaps);

    //Bind the uniform group to the context pipelines
    for(auto &Pipeline : context::Get()->Pipelines)
    {
        //This allocates the descriptor sets based on the DS layouts of each pipeline
        gfx::context::Get()->BindUniformsToPipeline(this->Uniforms, Pipeline.second, SceneDescriptorSetBinding);
    }    
    this->Uniforms->Update();
}

std::shared_ptr<object3D> scene::Clone(b8 CloneUUID)
{
    std::shared_ptr<scene> Result = std::make_shared<scene>(this->Name);

    Result->RenderOrder=this->RenderOrder;
    Result->FrustumCulled=this->FrustumCulled;
    Result->CastShadow=this->CastShadow;
    Result->ReceiveShadow=this->ReceiveShadow;
    Result->Transform =  this->Transform;
    if(CloneUUID) Result->ID = this->ID;

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

    gfx::pipelineHandle NewPipeline = context::Get()->Project.Materials[Mesh->MaterialID]->PipelineHandle;
    if(this->Meshes.find(NewPipeline) == this->Meshes.end()) this->Meshes[NewPipeline] = {};
    this->Meshes[NewPipeline].push_back(MeshPtr);
}

void scene::AddObject(std::shared_ptr<object3D> Object)
{
    object3D::AddObject(Object);
    if(object3D::AddToScene) this->AddMeshesInObject(Object);
    if(object3D::AddToScene) this->AddLightsInObject(Object);
    this->SceneGUI->NodeClicked = Object;
}

void scene::AddMesh(std::shared_ptr<object3D> Object)
{
    std::shared_ptr<mesh> Mesh = std::dynamic_pointer_cast<mesh>(Object);
    if(Mesh)
    {
        std::shared_ptr<material> MeshMaterial = context::Get()->Project.Materials[Mesh->MaterialID];
        std::vector<std::shared_ptr<mesh>> &Meshes = this->Meshes[MeshMaterial->PipelineHandle]; 
        if(std::find(Meshes.begin(), Meshes.end(), Mesh) == Meshes.end())
        {
            this->Meshes[MeshMaterial->PipelineHandle].push_back(Mesh);
            Mesh->MeshSceneID = this->Instances.size();
            this->Instances.push_back(Mesh);
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

void scene::AddLight(std::shared_ptr<object3D> Object)
{
    std::shared_ptr<light> Light = std::dynamic_pointer_cast<light>(Object);
    if(Light)
    {
        if(std::find(Lights, Lights + MaxLights, Light) == (Lights + MaxLights))
        {
            u32 AddInx = SceneBufferData.LightCount++;
            Light->IndexInScene = AddInx;
            this->Lights[AddInx] = Light;
            UpdateLight(AddInx);
        }
    }
}

void scene::UpdateLight(u32 Inx)
{
    this->SceneBufferData.Lights[Inx] = Lights[Inx]->Data;
    gfx::context::Get()->CopyDataToBuffer(this->SceneBuffer, &this->SceneBufferData, sizeof(v4i), 0);
    gfx::context::Get()->CopyDataToBuffer(this->SceneBuffer, &this->SceneBufferData.Lights[Inx], sizeof(light::lightData), sizeof(v4i) +  Inx * sizeof(light::lightData));
}

void scene::UpdateLightsAfter(u32 Index)
{
    gfx::context::Get()->CopyDataToBuffer(this->SceneBuffer, &this->SceneBufferData, sizeof(sceneBuffer), 0);
}

void scene::AddLightsInObject(std::shared_ptr<object3D> Object)
{
    // If it's a Light, store it in the Lights map
    AddLight(Object);

    for (sz i = 0; i < Object->Children.size(); i++)
    {
        AddLightsInObject(Object->Children[i]);
    }
}

void scene::OnRender(std::shared_ptr<camera> Camera)
{
    UpdateLightsAfter(0);
    
    for(auto &PipelineMeshes : this->Meshes)
    {
        if (PipelineMeshes.second.size() == 0) continue;
        std::shared_ptr<gfx::commandBuffer> CommandBuffer = gfx::context::Get()->GetCurrentFrameCommandBuffer();
        
        if(this->OverrideMaterial != nullptr) CommandBuffer->BindGraphicsPipeline(this->OverrideMaterial->PipelineHandle);
        else CommandBuffer->BindGraphicsPipeline(PipelineMeshes.first);
        
        CommandBuffer->BindUniformGroup(Camera->Uniforms, CameraDescriptorSetBinding);
        CommandBuffer->BindUniformGroup(this->Uniforms, SceneDescriptorSetBinding);
        for(sz i=0; i<PipelineMeshes.second.size(); i++)
        {
            PipelineMeshes.second[i]->OnRender(Camera);
        }
    }
}
void scene::OnAfterRender(std::shared_ptr<camera> Camera)
{
    // Update lights
    for (sz i = 0; i < this->SceneBufferData.LightCount; i++)
    {
        if(this->Lights[i]->Transform.HasChanged)
        {
            this->SceneBufferData.Lights[i] = this->Lights[i]->Data;
            UpdateLight(i);
            this->Lights[i]->Transform.HasChanged=false;
        }
    }

    object3D::OnAfterRender(Camera);


    while(ObjectDeletionQueue.size() > 0)
    {
        gfx::context::Get()->WaitIdle();
        std::shared_ptr<object3D> Object = ObjectDeletionQueue.top();
        ObjectDeletionQueue.pop();
        //Go through all children and delete them recursively
        if(this->SceneGUI->NodeClicked == Object) this->SceneGUI->NodeClicked = nullptr;
        this->RemoveBLASInstancesInObject(Object);

        ClearObject(Object);
        Object->Parent->DeleteChild(Object);    
    }

    if(context::UseRTX && (QueueBuildTLAS || InstancesToUpdate.size() > 0)) 
    {

        
        gfx::context::Get()->WaitIdle();
        if(this->TLAS == gfx::InvalidHandle) //Build tlas for the first time
            this->BuildTLAS();
        else if(InstancesToUpdate.size() > 0) //Only update instance positions
        {
            std::vector<glm::mat4*> Transforms(InstancesToUpdate.size());
            for(int i=0; i<InstancesToUpdate.size(); i++)
            {
                Transforms[i] = &this->Instances[InstancesToUpdate[i]]->Transform.Matrices.LocalToWorld;
            }
            gfx::context::Get()->UpdateAccelerationStructureInstances(this->TLAS, InstancesToUpdate, Transforms);
            InstancesToUpdate.clear();
        }
        else //Rebuilt the whole TLAS
            this->RebuildTLAS();
        
        this->BuildGlobalGeometryBuffers(); //Rebuild the global buffers accessed in the closest hit shader
        
        context::Get()->MainRenderer->SceneUpdate(); // Update the uniforms that use the AS

        QueueBuildTLAS=false;
        // for(auto *UniformGroup : gfx::uniformGroup::AllUniforms)
        // {
        //     for(auto &Uniform : UniformGroup->Uniforms)
        //     {
        //         if(Uniform.ResourceHandle == this->TLAS)
        //         {
        //             UniformGroup->Update();
        //         }
        //     }
        // }        
    }
}

void scene::DrawGUI()
{
    this->SceneGUI->DrawGUI();
}

void scene::BuildGlobalGeometryBuffers()
{
    if(!context::UseRTX) return;

    std::vector<vertex> Vertices;
    std::vector<u32> Triangles;
    std::vector<u32> GeometryOffsets;

    u32 IndexBase = 0;
    for(auto &Geometry :  context::Get()->Project.Geometries)
    {
        u32 VertexBase = Vertices.size();

        GeometryOffsets.push_back(IndexBase ) ;

        for(auto &Index : Geometry->IndexData)
        {
            Triangles.push_back(VertexBase + Index);
            IndexBase++;
        }

        for(auto &Vertex : Geometry->VertexData)
        {
            Vertices.push_back(Vertex);
        }
    }

    std::vector<u32> MaterialIDs;
    for(auto &Instance : this->Instances)
    {
        MaterialIDs.push_back(Instance->MaterialID);
    }



    this->InstanceMaterialIndices = gfx::context::Get()->CreateBuffer(MaterialIDs.size() * sizeof(u32), gfx::bufferUsage::StorageBuffer, gfx::memoryUsage::GpuOnly);
    gfx::context::Get()->CopyDataToBuffer(this->InstanceMaterialIndices, MaterialIDs.data(), MaterialIDs.size() * sizeof(u32), 0);    

    this->IndexBuffer = gfx::context::Get()->CreateBuffer(Triangles.size() * sizeof(u32), gfx::bufferUsage::StorageBuffer, gfx::memoryUsage::GpuOnly);
    gfx::context::Get()->CopyDataToBuffer(this->IndexBuffer, Triangles.data(), Triangles.size() * sizeof(u32), 0);    

    this->VertexBuffer = gfx::context::Get()->CreateBuffer(Vertices.size() * sizeof(vertex), gfx::bufferUsage::StorageBuffer, gfx::memoryUsage::GpuOnly);
    gfx::context::Get()->CopyDataToBuffer(this->VertexBuffer, Vertices.data(), Vertices.size() * sizeof(vertex), 0);        

    this->OffsetsBuffer = gfx::context::Get()->CreateBuffer(GeometryOffsets.size() * sizeof(u32), gfx::bufferUsage::StorageBuffer, gfx::memoryUsage::GpuOnly);
    gfx::context::Get()->CopyDataToBuffer(this->OffsetsBuffer, GeometryOffsets.data(), GeometryOffsets.size() * sizeof(u32), 0); 
}

void scene::UpdateGlobalMaterialBuffer()
{
    if(!context::UseRTX) return;

    std::vector<u32> MaterialIDs;
    for(auto &Instance : this->Instances)
    {
        MaterialIDs.push_back(Instance->MaterialID);
    }

    gfx::context::Get()->CopyDataToBuffer(this->InstanceMaterialIndices, MaterialIDs.data(), MaterialIDs.size() * sizeof(u32), 0);
 }

void scene::BuildTLAS()
{
    if(!context::UseRTX) return;

	std::vector<glm::mat4> Transforms;
    std::vector<s32> Instances;
    std::vector<gfx::accelerationStructureHandle> BLAS;
    std::vector<u32> MaterialIDs;

    u32 i=0;
    for(auto &Instance : this->Instances)
    {
        Instance->MeshSceneID = i++;
        Transforms.push_back(Instance->Transform.Matrices.LocalToWorld);
        Instances.push_back(Instance->GeometryID);
        MaterialIDs.push_back(Instance->MaterialID);
    }


    for(auto &Geometry : context::Get()->Project.Geometries)
    {
        BLAS.push_back(Geometry->BLAS);
    }

    TLAS = gfx::context::Get()->CreateTLAccelerationStructure(Transforms, BLAS, Instances);   
}

void scene::RebuildTLAS()
{
    if(!context::UseRTX) return;

	std::vector<glm::mat4> Transforms;
    std::vector<s32> Instances;
    std::vector<gfx::accelerationStructureHandle> BLAS;
    std::vector<u32> MaterialIDs;

    u32 i=0;
    for(auto &Instance : this->Instances)
    {
        Instance->MeshSceneID = i++;
        Transforms.push_back(Instance->Transform.Matrices.LocalToWorld);
        Instances.push_back(Instance->GeometryID);
        MaterialIDs.push_back(Instance->MaterialID);
    }

    gfx::context::Get()->DestroyBuffer(this->InstanceMaterialIndices);
    this->InstanceMaterialIndices = gfx::context::Get()->CreateBuffer(MaterialIDs.size() * sizeof(u32), gfx::bufferUsage::StorageBuffer, gfx::memoryUsage::GpuOnly);
    gfx::context::Get()->CopyDataToBuffer(this->InstanceMaterialIndices, MaterialIDs.data(), MaterialIDs.size() * sizeof(u32), 0);    

    for(auto &Geometry : context::Get()->Project.Geometries)
    {
        BLAS.push_back(Geometry->BLAS);
    }

    gfx::context::Get()->UpdateTLAccelerationStructure(TLAS , Transforms, BLAS, Instances);   
}

void scene::UpdateBLASInstance(u32 Index)
{
    if(!context::UseRTX) return;

    InstancesToUpdate.push_back(Index);
}



std::vector<std::shared_ptr<mesh>> GetInstanceInObject(std::shared_ptr<object3D> Object)
{
    std::vector<std::shared_ptr<mesh>> Result;
    std::shared_ptr<mesh> Mesh = std::dynamic_pointer_cast<mesh>(Object);
    if(Mesh) Result.push_back(Mesh);
        
    for(auto &Child : Object->Children)
    {
        std::shared_ptr<mesh> Mesh = std::dynamic_pointer_cast<mesh>(Child);
        if(Mesh) Result.push_back(Mesh);
        std::vector<std::shared_ptr<mesh>> ChildMeshes = GetInstanceInObject(Child);
        for(auto &ChildMesh : ChildMeshes)
        {
            Result.push_back(ChildMesh);
        }
    }
    return Result;
}

void scene::RemoveBLASInstancesInObject(std::shared_ptr<object3D> Object)
{
    // We need to find the index of each mesh in the global instances buffer
    std::vector<std::shared_ptr<mesh>> Instances = GetInstanceInObject(Object);
    for(auto &Instance : Instances)
    {
        this->Instances.erase(
            std::remove(this->Instances.begin(), this->Instances.end(), Instance), this->Instances.end());
    }
    u32 i=0;
    for(auto &Instance : Instances)
    {
        Instance->MeshSceneID = i++;
    }
    this->QueueBuildTLAS=true;
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

void scene::QueueDeleteObject(std::shared_ptr<object3D> Object)
{
    ObjectDeletionQueue.push(Object);
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

    FileStream.write((char*)&this->ID, sizeof(u32));
    
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

scene::~scene()
{
    printf("Destroying Scene\n");
    if(this->TLAS != gfx::InvalidHandle) gfx::context::Get()->DestroyAccelerationStructure(this->TLAS);
    if(this->VertexBuffer != gfx::InvalidHandle) gfx::context::Get()->DestroyBuffer(this->VertexBuffer);
    if(this->IndexBuffer != gfx::InvalidHandle) gfx::context::Get()->DestroyBuffer(this->IndexBuffer);
    if(this->InstanceMaterialIndices != gfx::InvalidHandle) gfx::context::Get()->DestroyBuffer(this->InstanceMaterialIndices);
    if(this->OffsetsBuffer != gfx::InvalidHandle) gfx::context::Get()->DestroyBuffer(this->OffsetsBuffer);
    this->TLAS = gfx::InvalidHandle;
    this->VertexBuffer = gfx::InvalidHandle;
    this->IndexBuffer = gfx::InvalidHandle;
    this->InstanceMaterialIndices = gfx::InvalidHandle;
    this->OffsetsBuffer = gfx::InvalidHandle;    
    gfx::context::Get()->QueueDestroyBuffer(this->SceneBuffer);
}

}