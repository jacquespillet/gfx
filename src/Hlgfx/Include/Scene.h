#pragma once
#include "Object3D.h"
#include "Camera.h"
#include "Mesh.h"
#include "Light.h"
#include "Bindings.h"
#include "Gfx/Include/Uniform.h"
#include "Context.h"
#include <stack>
namespace hlgfx
{
struct sceneGUI;

struct scene : public object3D
{
    scene(std::string Name = "Scene");
    ~scene();
    virtual void OnRender(std::shared_ptr<camera> Camera);
    virtual void AddObject(std::shared_ptr<object3D> Object) override;
    void AddMeshesInObject(std::shared_ptr<object3D> Object);
    virtual void OnAfterRender(std::shared_ptr<camera> Camera) override;
    void AddMesh(std::shared_ptr<object3D> Object);
    void AddLightsInObject(std::shared_ptr<object3D> Object);
    void AddLight(std::shared_ptr<object3D> Object);

    void UpdateMeshPipeline(gfx::pipelineHandle OldPipelineHandle, mesh *Mesh);
    
    std::unordered_map<gfx::pipelineHandle, std::vector<std::shared_ptr<mesh>>> Meshes;
    std::vector<std::shared_ptr<mesh>> Instances;

    std::vector<gfx::accelerationStructureHandle> BLAS;

    gfx::accelerationStructureHandle TLAS= gfx::InvalidHandle;
    gfx::bufferHandle VertexBuffer = gfx::InvalidHandle;
    gfx::bufferHandle IndexBuffer = gfx::InvalidHandle;
    gfx::bufferHandle InstanceMaterialIndices = gfx::InvalidHandle;
    gfx::bufferHandle OffsetsBuffer = gfx::InvalidHandle;


    void BuildTLAS();
    void RebuildTLAS();
    void BuildGlobalGeometryBuffers();
    std::vector<u32> InstancesToUpdate;
    void UpdateBLASInstance(u32 Index);
    void RemoveBLASInstancesInObject(std::shared_ptr<object3D> Object);
    b8 QueueBuildTLAS=false;
    

    virtual std::shared_ptr<object3D> Clone(b8 CloneUUID) override;

    struct sceneBuffer
    {
        float LightCount = 0;
        float MaxLightsCount = MaxLights;
        float ShadowMapSize = context::ShadowMapSize;
        float ShadowBias = 0.006f;
        light::lightData Lights[MaxLights];
    } SceneBufferData;
    std::shared_ptr<light> Lights[MaxLights];
    std::shared_ptr<gfx::uniformGroup> Uniforms;

    gfx::bufferHandle SceneBuffer;
    void UpdateLight(u32 Index);
    void UpdateLightsAfter(u32 Index);

    std::shared_ptr<material> OverrideMaterial = nullptr;

    virtual void DrawGUI() override;
    std::shared_ptr<sceneGUI> SceneGUI;

    void Clear();
    
    void QueueDeleteObject(std::shared_ptr<object3D> Object);
    std::stack<std::shared_ptr<object3D>> ObjectDeletionQueue;

    virtual void Serialize(std::string FilePath) override;
    virtual void Serialize(std::ofstream &FileStream) override;
};

}