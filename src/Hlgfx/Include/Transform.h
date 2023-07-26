#pragma once
#include "Types.h"
#include <vector>
namespace hlgfx
{

enum class rotationOrder
{
    xyz,
    xzy,
    yxz,
    yzx,
    zxy,
    zyx
};

//
struct transform
{
    transform();
    transform *Parent;
    std::vector<transform*> Children;

    v3f LocalPosition;
    v3f LocalRotation;
    quat LocalRotationQuat;
    v3f LocalScale;

    // v3f PositionInParent;
    // v3f RotationInParent;
    // v3f ScaleInParent;

    m4x4 LocalToWorld;
    m4x4 LocalToWorldNormal;
    m4x4 WorldToLocal;

    m4x4 LocalToParent;
    m4x4 LocalToParentNormal;
    m4x4 ParentToLocal;

    rotationOrder RotationOrder;

    void CalculateMatrices();
    void CalculateLocalToWorldMatrix();

    void SetParent(transform *Parent);

    void SetModelMatrix(m4x4 Matrix);
    void SetLocalPosition(v3f LocalPosition);
    void SetLocalRotation(v3f LocalRotation);
    void SetLocalScale(v3f LocalScale);

    void Translate(v3f Translation);
    void Rotate(v3f Rotation);
    void Scale(v3f Scale);

    void LookAt(v3f Position, v3f Target, v3f Up);

    v3f GetWorldPosition();
    quat GetWorldRotation();
    v3f GetWorldScale();

    b8 HasChanged=false;

    b8 MatrixBased=false;
};

}