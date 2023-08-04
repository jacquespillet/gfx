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
    std::unordered_map<gfx::pipelineHandle, std::vector<std::shared_ptr<mesh>>> Meshes;
    
    virtual void DrawGUI() override;
    void DrawNodeChildren(hlgfx::object3D *Object);
    void DrawSceneGUI();

    void Clear();
    void DeleteObject(std::shared_ptr<object3D> Object);
    
    void LoadFromFile(const char *FileName);
    void SaveToFile(const char *FileName);
    std::shared_ptr<hlgfx::object3D> NodeClicked = nullptr;
    u32 GuiWidth = 400;
};

}