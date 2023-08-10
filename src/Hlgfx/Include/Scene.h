#pragma once
#include "Object3D.h"
#include "Camera.h"
#include "Mesh.h"
#include "Light.h"
#include "Gfx/Include/Uniform.h"
#include <stack>
namespace hlgfx
{
struct sceneGUI;

struct scene : public object3D
{
    scene(std::string Name = "Scene");
    virtual void OnRender(std::shared_ptr<camera> Camera);
    virtual void AddObject(std::shared_ptr<object3D> Object) override;
    void AddMeshesInObject(std::shared_ptr<object3D> Object);
    virtual void OnAfterRender(std::shared_ptr<camera> Camera) override;
    void AddMesh(std::shared_ptr<object3D> Object);
    void AddLightsInObject(std::shared_ptr<object3D> Object);
    void AddLight(std::shared_ptr<object3D> Object);

    void UpdateMeshPipeline(gfx::pipelineHandle OldPipelineHandle, mesh *Mesh);
    std::unordered_map<gfx::pipelineHandle, std::vector<std::shared_ptr<mesh>>> Meshes;
    
    virtual std::shared_ptr<object3D> Clone(b8 CloneUUID) override;

    struct sceneBuffer
    {
        s32 LightCount = 0;
        v3i Padding0;
        light::lightData Lights[32];
    } SceneBufferData;
    std::shared_ptr<light> Lights[32];
    std::stack<u32> FreeIndices;
    std::shared_ptr<gfx::uniformGroup> Uniforms;

    gfx::bufferHandle SceneBuffer;
    void UpdateLight(u32 Index);

    virtual void DrawGUI() override;
    std::shared_ptr<sceneGUI> SceneGUI;

    void Clear();
    void DeleteObject(std::shared_ptr<object3D> Object);
    
    virtual void Serialize(std::string FilePath) override;
    virtual void Serialize(std::ofstream &FileStream) override;
};

}