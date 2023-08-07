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
    virtual std::shared_ptr<object3D> Clone() override;

    b8 MaterialSelectionOpen = false;
    std::shared_ptr<material> SelectedMaterial = nullptr;
    void ShowMaterialSelection(std::shared_ptr<material> &Material);
    virtual void DrawMaterial() override;
    //Api data
    std::shared_ptr<material> Material;
    std::shared_ptr<indexedGeometryBuffers> GeometryBuffers;
    struct uniformData
    {
        m4x4 ModelMatrix;
    } UniformData;
    gfx::bufferHandle UniformBuffer;
    std::shared_ptr<gfx::uniformGroup> Uniforms;
};
}