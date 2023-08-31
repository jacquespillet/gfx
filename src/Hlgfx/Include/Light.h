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
    void UpdateViewBox(std::shared_ptr<camera> Camera);
    v4f CalculateLightSpaceFrustumCorner(v3f StartPoint, v3f Direction, f32 Width);

    v3f ShadowMapViewport = v3f(30, 30, 100);
    f32 ShadowMapDistance = 10.0f;
    
    const f32 ShadowDistance = 20;
    u32 IndexInScene = 0;

    v3f BoxMin, BoxMax;
    f32 FarWidth, FarHeight, NearWidth, NearHeight;

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

    std::shared_ptr<camera> ShadowCam;
};

}