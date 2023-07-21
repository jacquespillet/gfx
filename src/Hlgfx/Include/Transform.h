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
    v3f LocalScale;

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

    void SetLocalPosition(v3f LocalPosition);
    void SetLocalRotation(v3f LocalRotation);
    void SetLocalScale(v3f LocalScale);

    void Translate(v3f Translation);
    void Rotate(v3f Rotation);
    void Scale(v3f Scale);

    void LookAt(v3f Position, v3f Target, v3f Up);
};

}