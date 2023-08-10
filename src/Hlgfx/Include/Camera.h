#pragma once
#include "Object3D.h"
#include "gfx/Include/Types.h"
#include "gfx/Include/Uniform.h"

namespace hlgfx
{
struct orbitCameraController;

struct camera : public object3D
{
    camera(f32 FOV, f32 AspectRatio, f32 NearClip = 0.01f, f32 FarClip = 100.0f);
    ~camera();
    struct cameraUniformData
    {
        f32 FOV;
        f32 AspectRatio;
        f32 NearClip;
        f32 FarClip;

        m4x4 ProjectionMatrix;
        m4x4 ViewMatrix;
        m4x4 ViewProjectionMatrix;

        v4f CameraPosition;
    } Data;

    void RecalculateMatrices();

    void SetLocalPosition(v3f LocalPosition);
    void SetLocalRotation(v3f LocalRotation);
    void SetLocalScale(v3f LocalScale);

    
    std::shared_ptr<hlgfx::orbitCameraController> Controls;
    
    gfx::bufferHandle UniformBuffer;
    std::shared_ptr<gfx::uniformGroup> Uniforms;
};
}