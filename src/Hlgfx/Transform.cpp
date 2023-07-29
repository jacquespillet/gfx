#include "Include/Transform.h"

#include <glm/ext.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

namespace hlgfx
{

b8 transform::DoCompute = true;


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

    this->LocalValues.LocalPosition = v3f(0,0,0);
    this->LocalValues.LocalRotation = v3f(0,0,0);
    this->LocalValues.LocalRotationQuat = glm::quat(this->LocalValues.LocalRotation);
    this->LocalValues.LocalScale = v3f(1,1,1);

    this->Matrices.LocalToWorld = m4x4(1);
    this->Matrices.LocalToWorldNormal = m4x4(1);
    this->Matrices.WorldToLocal = m4x4(1);

    this->Matrices.LocalToParent = m4x4(1);
    this->Matrices.LocalToParentNormal = m4x4(1);
    this->Matrices.ParentToLocal = m4x4(1);    
}

void transform::CalculateMatrices()
{
    if(!DoCompute) return;

    m4x4 UnitMatrix(1.0f);

    if(!MatrixBased)
    {
        //Local to parent
        m4x4 TranslationMatrix = glm::translate(UnitMatrix, this->LocalValues.LocalPosition);
        m4x4 ScaleMatrix = glm::scale(UnitMatrix, this->LocalValues.LocalScale);
        m4x4 RotationMatrix = glm::toMat4(this->LocalValues.LocalRotationQuat);
        this->Matrices.LocalToParent = TranslationMatrix * RotationMatrix * ScaleMatrix;
        this->Matrices.ParentToLocal = glm::inverse(this->Matrices.LocalToParent);
        this->Matrices.LocalToParentNormal = glm::inverseTranspose(this->Matrices.LocalToParent);
    }

    CalculateLocalToWorldMatrix();

    for (sz i = 0; i < Children.size(); i++)
    {
        Children[i]->CalculateLocalToWorldMatrix();
    }

}

void transform::CalculateLocalToWorldMatrix()
{
    if(!DoCompute) return;

    if(Parent != nullptr)
    {
        this->Matrices.LocalToWorld = Parent->Matrices.LocalToWorld * this->Matrices.LocalToParent;
    }
    else
    {
        this->Matrices.LocalToWorld = this->Matrices.LocalToParent;
    }
    this->Matrices.WorldToLocal = glm::inverse(this->Matrices.LocalToWorld);
    this->Matrices.LocalToWorldNormal = glm::inverseTranspose(this->Matrices.LocalToWorld);
    this->HasChanged=true;  

    for (sz i = 0; i < this->Children.size(); i++)
    {
        this->Children[i]->CalculateLocalToWorldMatrix();
    }
    
}

void transform::SetParent(transform *Parent)
{
    if(!DoCompute) 
    {
        this->Parent = Parent;
        return;
    }
    
    v3f ScaleDifference = this->GetWorldScale() / Parent->GetWorldScale();
    this->LocalValues.LocalScale = ScaleDifference;
    
    quat ThisRotation = this->GetWorldRotation();
    quat ParentRotation = Parent->GetWorldRotation();
    this->LocalValues.LocalRotationQuat = glm::inverse(ParentRotation) * (ThisRotation);

    v3f PositionDifference = this->GetWorldPosition() - Parent->GetWorldPosition();
    this->LocalValues.LocalPosition = (glm::inverse(ParentRotation) * PositionDifference) * this->LocalValues.LocalScale;

    this->Parent = Parent;

    CalculateMatrices();  

    HasChanged=true;    
}

void transform::SetModelMatrix(m4x4 Matrix)
{
    this->MatrixBased=true;
    this->Matrices.LocalToParent = Matrix;
    this->Matrices.ParentToLocal = glm::inverse(this->Matrices.LocalToParent);
    this->Matrices.LocalToParentNormal = glm::inverseTranspose(this->Matrices.LocalToParent);
    CalculateMatrices();
    this->HasChanged=true;
}

void transform::SetLocalPosition(v3f LocalPosition)
{
    this->MatrixBased=false;
    this->LocalValues.LocalPosition = LocalPosition;
    CalculateMatrices();
    HasChanged=true;
}

void transform::SetLocalRotation(v3f LocalRotation)
{
    this->MatrixBased=false;
    this->LocalValues.LocalRotation = LocalRotation;
    this->LocalValues.LocalRotationQuat = glm::quat(glm::radians(this->LocalValues.LocalRotation));
    CalculateMatrices();
    HasChanged=true;
}

void transform::SetLocalScale(v3f LocalScale)
{
    this->MatrixBased=false;
    this->LocalValues.LocalScale = LocalScale;
    CalculateMatrices();
    HasChanged=true;
}

void transform::Translate(v3f Translation)
{
    this->MatrixBased=false;
    this->LocalValues.LocalPosition += Translation;
    CalculateMatrices();
    HasChanged=true;
}
void transform::Rotate(v3f Rotation)
{
    this->MatrixBased=false;
    this->LocalValues.LocalRotation += Rotation;
    CalculateMatrices();
    HasChanged=true;
}

void transform::Scale(v3f Scale)
{
    this->MatrixBased=false;
    this->LocalValues.LocalScale += Scale;
    CalculateMatrices();
    HasChanged=true;
}


v3f transform::GetWorldPosition()
{
    return v3f(this->Matrices.LocalToWorld * v4f(0,0,0,1));
}

quat transform::GetWorldRotation()
{
    std::vector<quat> Quaternions;
    transform *Transform = this;
    while(true)
    {
        Quaternions.push_back(Transform->LocalValues.LocalRotationQuat);
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
    glm::decompose(Matrices.LocalToWorld, scale, rotation, translation, skew,perspective);    
    return scale;
}



void transform::LookAt(v3f Position, v3f Target, v3f Up)
{

}

}