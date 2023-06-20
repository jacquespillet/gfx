#pragma once

namespace gfx
{
struct swapchain
{
    void Present();

    void *ApiData;
};
}