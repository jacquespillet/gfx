#pragma once
#include "Gfx/Include/Types.h"
#include <memory>

namespace hlgfx
{
struct scene;
struct camera;
struct material;
struct renderer;


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

struct shadowsRenderer : public renderer
{
    virtual void Render(std::shared_ptr<scene> Scene, std::shared_ptr<camera> Camera) override;
};



}