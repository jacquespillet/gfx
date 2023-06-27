#pragma once
#include <memory>

namespace gfx
{
struct swapchain
{
    void Present();

    uint32_t Width, Height;
    
    std::shared_ptr<void> ApiData;
};
}