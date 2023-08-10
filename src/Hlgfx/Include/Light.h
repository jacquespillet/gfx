#pragma once
#include "Object3D.h"

namespace hlgfx
{
struct light : public object3D
{
    light(std::string Name);
    virtual void OnUpdate() override;
    enum lightType
    {
        Point,
        Directional,
        Spot,
        Area
    };

    struct lightData
    {
        v3f Color;
        f32 Intensity;
        
        v3f Size;
        f32 Type;
        
        v3f Position;
        f32 Padding1;

        v3f Direction;
        f32 Padding2;
    } Data;
};

}