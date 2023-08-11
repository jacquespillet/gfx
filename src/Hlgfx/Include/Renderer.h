#pragma once
#include "Gfx/Include/Types.h"
#include <memory>

namespace hlgfx
{
struct scene;
struct camera;
struct material;
struct renderer
{
    gfx::framebufferHandle RenderTarget;
    std::shared_ptr<material> OverrideMaterial = nullptr;
    void Render(std::shared_ptr<scene> Scene, std::shared_ptr<camera> Camera);
};
}