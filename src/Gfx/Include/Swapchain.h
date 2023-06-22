#pragma once

namespace gfx
{
struct swapchain
{
    void Present();

    uint32_t Width, Height;
    
    void *ApiData;
};
}