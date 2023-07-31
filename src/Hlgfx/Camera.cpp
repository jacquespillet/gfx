#include "gfx/Include/Context.h"
#include "Include/Camera.h"
#include "Include/Context.h"
#include "Include/CameraController.h"
#include "Include/Bindings.h"
#include <glm/ext.hpp>
namespace hlgfx
{
camera::camera(f32 FOV, f32 AspectRatio, f32 NearClip, f32 FarClip) : object3D("Camera")
{
    this->Controls = std::make_shared<orbitCameraController>(this);

    gfx::context *LowLevelContext = gfx::context::Get();
    context *HighLevelContext = context::Get();
    
    //Create uniform buffer and uniform group
    this->UniformBuffer = LowLevelContext->CreateBuffer(sizeof(cameraUniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
    this->Uniforms = std::make_shared<gfx::uniformGroup>();
    this->Uniforms->Reset();
    this->Uniforms->AddUniformBuffer(CameraBinding, this->UniformBuffer);

    this->Data.FOV = FOV;
    this->Data.AspectRatio = AspectRatio;
    this->Data.NearClip = NearClip;
    this->Data.FarClip = FarClip;
    RecalculateMatrices();

    //Bind the uniform group to the context pipelines
    for(auto &Pipeline : HighLevelContext->Pipelines)
    {
        //This allocates the descriptor sets based on the DS layouts of each pipeline
        LowLevelContext->BindUniformsToPipeline(this->Uniforms, Pipeline.second, CameraDescriptorSetBinding);
    }
    this->Uniforms->Update();

}

void camera::SetLocalPosition(v3f LocalPosition)
{
    this->Transform.SetLocalPosition(LocalPosition);
    RecalculateMatrices();
    this->Controls->Recalculate();
}
void camera::SetLocalRotation(v3f LocalRotation)
{
    this->Transform.SetLocalRotation(LocalRotation);
    RecalculateMatrices();
    this->Controls->Recalculate();
}
void camera::SetLocalScale(v3f LocalScale)
{
    this->Transform.SetLocalScale(LocalScale);
    RecalculateMatrices();
    this->Controls->Recalculate();
}


void camera::RecalculateMatrices()
{
    this->Data.ProjectionMatrix = glm::perspective(glm::radians(this->Data.FOV), this->Data.AspectRatio, this->Data.NearClip, this->Data.FarClip);
    this->Data.ViewMatrix = this->Transform.Matrices.WorldToLocal;
    this->Data.ViewProjectionMatrix = this->Data.ProjectionMatrix * this->Data.ViewMatrix;
    gfx::context::Get()->CopyDataToBuffer(this->UniformBuffer, &this->Data, sizeof(cameraUniformData), 0);
}

camera::~camera()
{
    gfx::context::Get()->QueueDestroyBuffer(this->UniformBuffer);
}
}