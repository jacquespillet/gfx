#pragma once
#include "Object3D.h"
#include "Camera.h"
#include "Mesh.h"
#include "Gfx/Include/Uniform.h"

namespace hlgfx
{
struct sceneGUI;

struct scene : public object3D
{
    scene(std::string Name = "Scene");
    virtual void OnRender(std::shared_ptr<camera> Camera);
    virtual void AddObject(std::shared_ptr<object3D> Object) override;
    void AddMeshesInObject(std::shared_ptr<object3D> Object);
    void AddMesh(std::shared_ptr<object3D> Object);

    void UpdateMeshPipeline(gfx::pipelineHandle OldPipelineHandle, mesh *Mesh);
    std::unordered_map<gfx::pipelineHandle, std::vector<std::shared_ptr<mesh>>> Meshes;
    
    virtual std::shared_ptr<object3D> Clone(b8 CloneUUID) override;

    virtual void DrawGUI() override;
    std::shared_ptr<sceneGUI> SceneGUI;

    void Clear();
    void DeleteObject(std::shared_ptr<object3D> Object);
    
    virtual void Serialize(std::string FilePath) override;
    virtual void Serialize(std::ofstream &FileStream) override;
};

}