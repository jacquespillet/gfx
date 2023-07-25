#include "Include/CameraController.h"
#include "Include/Context.h"
#include <glm/ext.hpp>

namespace hlgfx
{

orbitCameraController::orbitCameraController(std::shared_ptr<camera> Camera) : object3D("CameraController")
{
    this->Camera = Camera;


    v3f Position = this->Camera->Transform.LocalPosition - this->Target;
    this->Distance = std::sqrt(Position.x * Position.x + Position.y * Position.y + Position.z * Position.z);
    this->Theta = std::acos(Position.y / this->Distance);
    this->Phi = std::atan2(Position.z, Position.x);

}

void orbitCameraController::OnUpdate()
{
    context *Context = context::Get();
    if(Context->IsInteractingGUI) return;

    b8 ShouldRecalculate=false;
    if(Context->LeftButtonPressed)
    {
        this->Phi += Context->MouseDelta.x * 0.001f * this->MouseSpeedX;
        this->Theta -= Context->MouseDelta.y * 0.001f * this->MouseSpeedY;
        ShouldRecalculate=true;
    }
    if(Context->MouseWheelChanged)
    {
        f32 Offset = Context->MouseWheelY * 0.1f * this->MouseSpeedWheel * this->Distance;
        this->Distance -= Offset;
        if(Distance < 0.1f) Distance = 0.1f;
        ShouldRecalculate=true;
    }


  
    if(ShouldRecalculate)
    {
        v3f Position;
        Position.x = this->Distance * std::sin(this->Theta) * std::cos(this->Phi);
        Position.z = this->Distance * std::sin(this->Theta) * std::sin(this->Phi);
        Position.y = this->Distance * std::cos(this->Theta);
        
        m4x4 LookAtMatrix = glm::lookAt(Position, this->Target, v3f(0,1,0));
        this->Camera->Transform.SetModelMatrix(glm::inverse(LookAtMatrix));
        this->Camera->RecalculateMatrices();
    }
    
}

}