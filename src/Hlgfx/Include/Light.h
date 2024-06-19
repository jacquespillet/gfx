#pragma once
#include "Object3D.h"
#include <fstream>
#include "Gfx/Include/Framebuffer.h"

namespace hlgfx
{
struct material;

struct light : public object3D
{
    enum lightType
    {
        Point,
        Directional,
        Spot,
        Area
    };
    
    light(std::string Name, lightType Type);
    ~light();
    virtual void OnUpdate() override;
    virtual void DrawCustomGUI() override;
    virtual void Serialize(std::ofstream &FileStream) override;
    void CalculateMatrices(std::shared_ptr<camera> Camera);

    u32 IndexInScene = 0;


    struct lightData
    {
        v4f ColorAndIntensity;
        v4f SizeAndType;
        v4f Position;
        v4f Direction;    
        m4x4 LightSpaceMatrix;    
    } Data;

    b8 AutomaticShadowFrustum=false;
    b8 UseMainCameraFarPlane=false;
    v3f ShadowFrustumSize = v3f(50);
    f32 ShadowDistance = 10.0f;    

    gfx::framebufferHandle ShadowsFramebuffer;
	gfx::pipelineHandle PipelineHandleOffscreen;
	std::shared_ptr<material> Material;

    std::shared_ptr<camera> ShadowCam;
};

}