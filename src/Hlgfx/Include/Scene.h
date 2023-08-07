#pragma once
#include "Object3D.h"
#include "Camera.h"
#include "Mesh.h"
#include "Gfx/Include/Uniform.h"

namespace hlgfx
{

struct scene : public object3D
{
    scene();
    virtual void OnRender(std::shared_ptr<camera> Camera);
    virtual void AddObject(std::shared_ptr<object3D> Object) override;
    void AddMeshesInObject(std::shared_ptr<object3D> Object);
    void UpdateMeshPipeline(gfx::pipelineHandle OldPipelineHandle, mesh *Mesh);
    std::unordered_map<gfx::pipelineHandle, std::vector<std::shared_ptr<mesh>>> Meshes;
    
    virtual void DrawGUI() override;
    void DrawNodeChildren(hlgfx::object3D *Object);
    void DrawSceneGUI();

    void Clear();
    void DeleteObject(std::shared_ptr<object3D> Object);
    
    virtual void Serialize(std::string FilePath) override;
    virtual void Serialize(std::ofstream &FileStream) override;

    std::shared_ptr<hlgfx::object3D> NodeClicked = nullptr;
    u32 GuiWidth = 400;
};

}