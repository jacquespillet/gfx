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
        v4f ColorAndIntensity;
        v4f SizeAndType;
        v4f Position;
        v4f Direction;        
    } Data;
};

}