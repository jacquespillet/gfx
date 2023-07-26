#include "Include/Transform.h"

#include <glm/ext.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace hlgfx
{

// m4x4 GetRotationMatrix(v3f EulerAngles, rotationOrder Order)
// {
//     m4x4 UnitMatrix(1.0f);

//     m4x4 RotationX = glm::rotate(UnitMatrix, glm::radians(EulerAngles.x), v3f(1,0,0));
//     m4x4 RotationY = glm::rotate(UnitMatrix, glm::radians(EulerAngles.y), v3f(0,1,0));
//     m4x4 RotationZ = glm::rotate(UnitMatrix, glm::radians(EulerAngles.z), v3f(1,0,1));

//     switch (Order)
//     {
//     case rotationOrder::xyz:
//         return RotationZ * RotationY * RotationX;
//         break;
//     case rotationOrder::xzy:
//         return RotationY * RotationZ * RotationX;
//         break;
//     case rotationOrder::yxz:
//         return RotationZ * RotationX * RotationY;
//         break;
//     case rotationOrder::yzx:
//         return RotationX * RotationZ * RotationY;
//         break;
//     case rotationOrder::zxy:
//         return RotationY * RotationX * RotationZ;
//         break;
//     case rotationOrder::zyx:
//         return RotationX * RotationY * RotationZ;
//         break;
//     default:
//         break;
//     }

//     assert(false);
// }

transform::transform()
{
    this->Parent = nullptr;
    this->Children.resize(0);

    this->LocalPosition = v3f(0,0,0);
    this->LocalRotation = v3f(0,0,0);
    this->LocalRotationQuat = glm::quat(this->LocalRotation);
    this->LocalScale = v3f(1,1,1);

    this->LocalToWorld = m4x4(1);
    this->LocalToWorldNormal = m4x4(1);
    this->WorldToLocal = m4x4(1);

    this->LocalToParent = m4x4(1);
    this->LocalToParentNormal = m4x4(1);
    this->ParentToLocal = m4x4(1);    
}

void transform::CalculateMatrices()
{
    m4x4 UnitMatrix(1.0f);

    if(!MatrixBased)
    {
        //Local to parent
        m4x4 TranslationMatrix = glm::translate(UnitMatrix, this->LocalPosition);
        m4x4 ScaleMatrix = glm::scale(UnitMatrix, this->LocalScale);
        m4x4 RotationMatrix = glm::toMat4(this->LocalRotationQuat);
        this->LocalToParent = TranslationMatrix * RotationMatrix * ScaleMatrix;
        this->ParentToLocal = glm::inverse(this->LocalToParent);
        this->LocalToParentNormal = glm::inverseTranspose(this->LocalToParent);
    }

    CalculateLocalToWorldMatrix();

    for (sz i = 0; i < Children.size(); i++)
    {
        Children[i]->CalculateLocalToWorldMatrix();
    }

}

void transform::CalculateLocalToWorldMatrix()
{
    if(Parent != nullptr)
    {
        this->LocalToWorld = Parent->LocalToWorld * this->LocalToParent;
    }
    else
    {
        this->LocalToWorld = this->LocalToParent;
    }
    this->WorldToLocal = glm::inverse(this->LocalToWorld);
    this->LocalToWorldNormal = glm::inverseTranspose(this->LocalToWorld);
    this->HasChanged=true;  
}

void transform::SetParent(transform *Parent)
{

    v3f ScaleDifference = this->GetWorldScale() / Parent->GetWorldScale();
    this->LocalScale = ScaleDifference;
    
    quat ThisRotation = this->GetWorldRotation();
    quat ParentRotation = Parent->GetWorldRotation();
    this->LocalRotationQuat = glm::inverse(ParentRotation) * (ThisRotation);

    v3f PositionDifference = this->GetWorldPosition() - Parent->GetWorldPosition();
    this->LocalPosition = (glm::inverse(ParentRotation) * PositionDifference) * this->LocalScale;

    this->Parent = Parent;

    CalculateMatrices();  

    HasChanged=true;    
}

void transform::SetModelMatrix(m4x4 Matrix)
{
    this->MatrixBased=true;
    this->LocalToParent = Matrix;
    this->ParentToLocal = glm::inverse(this->LocalToParent);
    this->LocalToParentNormal = glm::inverseTranspose(this->LocalToParent);
    CalculateMatrices();
    this->HasChanged=true;
}

void transform::SetLocalPosition(v3f LocalPosition)
{
    this->MatrixBased=false;
    this->LocalPosition = LocalPosition;
    CalculateMatrices();
    HasChanged=true;
}

void transform::SetLocalRotation(v3f LocalRotation)
{
    this->MatrixBased=false;
    this->LocalRotation = LocalRotation;
    this->LocalRotationQuat = glm::quat(glm::radians(this->LocalRotation));
    CalculateMatrices();
    HasChanged=true;
}

void transform::SetLocalScale(v3f LocalScale)
{
    this->MatrixBased=false;
    this->LocalScale = LocalScale;
    CalculateMatrices();
    HasChanged=true;
}

void transform::Translate(v3f Translation)
{
    this->MatrixBased=false;
    this->LocalPosition += Translation;
    CalculateMatrices();
    HasChanged=true;
}
void transform::Rotate(v3f Rotation)
{
    this->MatrixBased=false;
    this->LocalRotation += Rotation;
    CalculateMatrices();
    HasChanged=true;
}

void transform::Scale(v3f Scale)
{
    this->MatrixBased=false;
    this->LocalScale += Scale;
    CalculateMatrices();
    HasChanged=true;
}


v3f transform::GetWorldPosition()
{
    return v3f(this->LocalToWorld * v4f(0,0,0,1));
}

quat transform::GetWorldRotation()
{
    std::vector<quat> Quaternions;
    transform *Transform = this;
    while(true)
    {
        Quaternions.push_back(Transform->LocalRotationQuat);
        if(Transform->Parent != nullptr)
        {
            Transform = Transform->Parent;
        }
        else break;
    }
    quat Result = Quaternions[Quaternions.size()-1];
    for(s32 i=Quaternions.size()-2; i>=0; i--)
    {
        Result *= Quaternions[i];
    }
    return Result;
}

v3f transform::GetWorldScale()
{
    glm::vec3 scale;
    glm::quat rotation;
    glm::vec3 translation;
    glm::vec3 skew;
    glm::vec4 perspective;
    glm::decompose(LocalToWorld, scale, rotation, translation, skew,perspective);    
    return scale;
}



void transform::LookAt(v3f Position, v3f Target, v3f Up)
{

}

}