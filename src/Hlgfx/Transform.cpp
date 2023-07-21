#include "Include/Transform.h"

#include <glm/ext.hpp>

namespace hlgfx
{

m4x4 GetRotationMatrix(v3f EulerAngles, rotationOrder Order)
{
    m4x4 UnitMatrix(1.0f);

    m4x4 RotationX = glm::rotate(UnitMatrix, EulerAngles.x, v3f(1,0,0));
    m4x4 RotationY = glm::rotate(UnitMatrix, EulerAngles.y, v3f(0,1,0));
    m4x4 RotationZ = glm::rotate(UnitMatrix, EulerAngles.z, v3f(1,0,1));

    switch (Order)
    {
    case rotationOrder::xyz:
        return RotationZ * RotationY * RotationX;
        break;
    case rotationOrder::xzy:
        return RotationY * RotationZ * RotationX;
        break;
    case rotationOrder::yxz:
        return RotationZ * RotationX * RotationY;
        break;
    case rotationOrder::yzx:
        return RotationX * RotationZ * RotationY;
        break;
    case rotationOrder::zxy:
        return RotationY * RotationX * RotationZ;
        break;
    case rotationOrder::zyx:
        return RotationX * RotationY * RotationZ;
        break;
    default:
        break;
    }

    assert(false);
}

transform::transform()
{
    this->Parent = nullptr;
    this->Children.resize(0);

    this->LocalPosition = v3f(0,0,0);
    this->LocalRotation = v3f(0,0,0);
    this->LocalScale = v3f(1,1,1);

    this->LocalToWorld = m4x4(1);
    this->LocalToWorldNormal = m4x4(1);
    this->WorldToLocal = m4x4(1);

    this->LocalToParent = m4x4(1);
    this->LocalToParentNormal = m4x4(1);
    this->ParentToLocal = m4x4(1);    

    this->RotationOrder = rotationOrder::zxy;
}

void transform::CalculateMatrices()
{
    m4x4 UnitMatrix(1.0f);

    //Local to parent
    m4x4 TranslationMatrix = glm::translate(UnitMatrix, this->LocalPosition);
    m4x4 ScaleMatrix = glm::scale(UnitMatrix, this->LocalScale);
    m4x4 RotationMatrix = GetRotationMatrix(this->LocalRotation, this->RotationOrder);
    this->LocalToParent = TranslationMatrix * RotationMatrix * ScaleMatrix;
    this->ParentToLocal = glm::inverse(this->LocalToParent);
    this->LocalToParentNormal = glm::inverseTranspose(this->LocalToParent);

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
        this->WorldToLocal = glm::inverse(this->LocalToWorld);
        this->LocalToWorldNormal = glm::inverseTranspose(this->LocalToWorld);
    }
}

void transform::SetParent(transform *Parent)
{
    this->Parent = Parent;
    
    v3f PositionDifference = this->LocalPosition - Parent->LocalPosition;
    this->LocalPosition -= PositionDifference;

    v3f RotationDifference = this->LocalRotation - Parent->LocalRotation;
    this->LocalRotation -= RotationDifference;

    v3f ScaleDifference = this->LocalScale - Parent->LocalScale;
    this->LocalScale -= ScaleDifference;

    CalculateMatrices();
}

void transform::SetLocalPosition(v3f LocalPosition)
{
    this->LocalPosition = LocalPosition;
    CalculateMatrices();
}

void transform::SetLocalRotation(v3f LocalRotation)
{
    this->LocalRotation = LocalRotation;
    CalculateMatrices();
}

void transform::SetLocalScale(v3f LocalScale)
{
    this->LocalScale = LocalScale;
    CalculateMatrices();
}

void transform::Translate(v3f Translation)
{
    this->LocalPosition += Translation;
    CalculateMatrices();
}
void transform::Rotate(v3f Rotation)
{
    this->LocalRotation += Rotation;
    CalculateMatrices();
}

void transform::Scale(v3f Scale)
{
    this->LocalScale += Scale;
    CalculateMatrices();
}


void transform::LookAt(v3f Position, v3f Target, v3f Up)
{

}

}