#pragma once
#include "Object3D.h"
#include "Geometry.h"
#include "Gfx/Api.h"
#include <string>
#include <fstream>

namespace hlgfx
{
struct material;
struct mesh : public object3D
{

    mesh(std::string Name);
    mesh();
    ~mesh();

    virtual void OnEarlyUpdate() override;
    virtual void OnRender(std::shared_ptr<camera> Camera) override;
    virtual void Serialize(std::ofstream &FileStream) override;
    virtual std::shared_ptr<object3D> Clone(b8 CloneUUID) override;

    b8 MaterialSelectionOpen = false;
    std::shared_ptr<material> SelectedMaterial = nullptr;
    void ShowMaterialSelection(std::shared_ptr<material> &Material);
    virtual void DrawMaterial() override;
    virtual void DrawCustomGUI() override;
    //Api data
    
    // std::shared_ptr<material> Material;
    // std::shared_ptr<indexedGeometryBuffers> GeometryBuffers;

    u32 MaterialID;
    u32 GeometryID;
    u32 MeshSceneID;


    struct uniformData
    {
        m4x4 ModelMatrix;
        m4x4 NormalMatrix;
        v4i Selected = v4i(0);
    } UniformData;
    gfx::bufferHandle UniformBuffer;
    std::shared_ptr<gfx::uniformGroup> Uniforms;
};
}