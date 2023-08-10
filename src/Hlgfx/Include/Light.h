#pragma once
#include "Object3D.h"
#include <fstream>

namespace hlgfx
{
struct light : public object3D
{
    light(std::string Name);
    virtual void OnUpdate() override;
    virtual void DrawCustomGUI() override;
    virtual void Serialize(std::ofstream &FileStream) override;
    
    u32 IndexInScene = 0;
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