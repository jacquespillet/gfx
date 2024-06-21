#pragma once
#include "Gfx/Include/Types.h"
#include <memory>

namespace hlgfx
{
struct scene;
struct camera;
struct material;
struct renderer;
struct indexedGeometryBuffers;



struct renderer
{
    gfx::framebufferHandle RenderTarget;
    std::shared_ptr<material> OverrideMaterial = nullptr;

    virtual void Render(std::shared_ptr<scene> Scene, std::shared_ptr<camera> Camera) = 0;
};

struct mainRenderer : public renderer
{
    virtual void Render(std::shared_ptr<scene> Scene, std::shared_ptr<camera> Camera) override;
};

struct deferredRenderer : public renderer
{   
    gfx::pipelineHandle CompositionPipeline;
    std::shared_ptr<material> CompositionMaterial = nullptr;

    std::shared_ptr<indexedGeometryBuffers> QuadGeometry;
 
    deferredRenderer();
    virtual void Render(std::shared_ptr<scene> Scene, std::shared_ptr<camera> Camera) override;
};

struct shadowsRenderer : public renderer
{
    virtual void Render(std::shared_ptr<scene> Scene, std::shared_ptr<camera> Camera) override;
};



}