#include "Include/Light.h"
#include <glm/ext.hpp>

namespace hlgfx
{

light::light(std::string Name) : object3D(Name)
{
    this->Data.Color = v3f(1,1,1);
    this->Data.Intensity = 10.0f;
    this->Data.Size = v3f(1,1,1);
    this->Data.Position = this->Transform.GetWorldPosition();
    this->Data.Direction = glm::column(this->Transform.Matrices.LocalToWorld, 2);
}

void light::OnUpdate()
{
    this->Data.Position = this->Transform.GetWorldPosition();
    this->Data.Direction = glm::column(this->Transform.Matrices.LocalToWorld, 2);
}
}