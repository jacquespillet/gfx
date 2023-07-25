#include "gfx/Include/Context.h"
#include "Include/Camera.h"
#include "Include/Context.h"
#include <glm/ext.hpp>

namespace hlgfx
{
camera::camera(f32 FOV, f32 AspectRatio, f32 NearClip, f32 FarClip) : object3D("Camera")
{
    gfx::context *LowLevelContext = gfx::context::Get();
    context *HighLevelContext = context::Get();
    
    //Create uniform buffer and uniform group
    this->UniformBuffer = LowLevelContext->CreateBuffer(sizeof(cameraUniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
    this->Uniforms = std::make_shared<gfx::uniformGroup>();
    this->Uniforms->Reset();
    this->Uniforms->AddUniformBuffer(0, this->UniformBuffer);

    this->Data.FOV = FOV;
    this->Data.AspectRatio = AspectRatio;
    this->Data.NearClip = NearClip;
    this->Data.FarClip = FarClip;
    RecalculateMatrices();

    //Bind the uniform group to the context pipelines
    for(auto &Pipeline : HighLevelContext->Pipelines)
    {
        //This allocates the descriptor sets based on the DS layouts of each pipeline
        LowLevelContext->BindUniformsToPipeline(this->Uniforms, Pipeline.second, CameraUniformsBinding);
    }
    this->Uniforms->Update();
}



void camera::RecalculateMatrices()
{
    this->Data.ProjectionMatrix = glm::perspective(glm::radians(this->Data.FOV), this->Data.AspectRatio, this->Data.NearClip, this->Data.FarClip);
    this->Data.ViewMatrix = this->Transform.WorldToLocal;
    this->Data.ViewProjectionMatrix = this->Data.ProjectionMatrix * this->Data.ViewMatrix;
    gfx::context::Get()->CopyDataToBuffer(this->UniformBuffer, &this->Data, sizeof(cameraUniformData), 0);
}
}