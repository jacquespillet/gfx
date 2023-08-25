#pragma once
#include "Object3D.h"
#include <fstream>
#include "Gfx/Include/Framebuffer.h"

namespace hlgfx
{
struct material;

struct light : public object3D
{
    light(std::string Name);
    ~light();
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
        m4x4 LightSpaceMatrix;    
    } Data;

    gfx::framebufferHandle ShadowsFramebuffer;
	gfx::pipelineHandle PipelineHandleOffscreen;
	std::shared_ptr<material> Material;
};

}