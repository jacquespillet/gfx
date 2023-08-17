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
    this->Ortho=false;
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
camera::camera(f32 Left, f32 Right, f32 Bottom, f32 Top, f32 Back, f32 Front) : object3D("OrthoCamera")
{
    this->Ortho=true;
    
    this->Controls = std::make_shared<orbitCameraController>(this);

    gfx::context *LowLevelContext = gfx::context::Get();
    context *HighLevelContext = context::Get();
    
    //Create uniform buffer and uniform group
    this->UniformBuffer = LowLevelContext->CreateBuffer(sizeof(cameraUniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
    this->Uniforms = std::make_shared<gfx::uniformGroup>();
    this->Uniforms->Reset();
    this->Uniforms->AddUniformBuffer(CameraBinding, this->UniformBuffer);

    this->SetOrtho(Left, Right, Bottom, Top, Back, Front);
    RecalculateMatrices();

    //Bind the uniform group to the context pipelines
    for(auto &Pipeline : HighLevelContext->Pipelines)
    {
        //This allocates the descriptor sets based on the DS layouts of each pipeline
        LowLevelContext->BindUniformsToPipeline(this->Uniforms, Pipeline.second, CameraDescriptorSetBinding);
    }
    this->Uniforms->Update();    
}

void camera::SetOrtho(f32 Left, f32 Right, f32 Bottom, f32 Top, f32 Back, f32 Front)
{
    this->Ortho = true;
    this->Data.LeftRightBottomTop.x = Left;
    this->Data.LeftRightBottomTop.y = Right;
    this->Data.LeftRightBottomTop.z = Bottom;
    this->Data.LeftRightBottomTop.w = Top;
    this->Data.BackFront.x = Back;
    this->Data.BackFront.y = Front;

    RecalculateMatrices();
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

m4x4 GetOrtho(f32 Left, f32 Right, f32 Bottom, f32 Top, f32 Near, f32 Far)
{
    m4x4 Ortho;
    
    Ortho[0][0] = 2.0f / (Right - Left);
    Ortho[0][1] = 0.0f;
    Ortho[0][2] = 0.0f;
    Ortho[0][3] = 0.0f;

    Ortho[1][0] =0.0f;
    Ortho[1][1] =2.0f / (Top - Bottom);
    Ortho[1][2] =0.0f;
    Ortho[1][3] =0.0f;

    Ortho[2][0] =0.0f;
    Ortho[2][1] =0.0f;
    Ortho[2][2] =1.0f / (Near - Far);
    Ortho[2][3] =0.0f;

    Ortho[3][0] =-(Right + Left) / (Right - Left);
    Ortho[3][1] =-(Bottom + Top) / (Top - Bottom);
    Ortho[3][2] = Near / (Near - Far);
    Ortho[3][3] =1.0f;
    return Ortho;
}


void camera::RecalculateMatrices()
{
    if(this->Ortho)
        this->Data.ProjectionMatrix = glm::ortho(this->Data.LeftRightBottomTop.x, this->Data.LeftRightBottomTop.y, this->Data.LeftRightBottomTop.z, this->Data.LeftRightBottomTop.w, this->Data.BackFront.x, this->Data.BackFront.y);
    else
        this->Data.ProjectionMatrix = glm::perspective(glm::radians(this->Data.FOV), this->Data.AspectRatio, this->Data.NearClip, this->Data.FarClip);
    this->Data.ViewMatrix = this->Transform.Matrices.WorldToLocal;
    this->Data.ViewProjectionMatrix = this->Data.ProjectionMatrix * this->Data.ViewMatrix;
    this->Data.CameraPosition = v4f(this->Transform.GetWorldPosition(), 1);
    gfx::context::Get()->CopyDataToBuffer(this->UniformBuffer, &this->Data, sizeof(cameraUniformData), 0);
}

camera::~camera()
{
    gfx::context::Get()->QueueDestroyBuffer(this->UniformBuffer);
}
}