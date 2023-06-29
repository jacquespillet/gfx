#pragma once

namespace gfx
{
struct commandBuffer;
struct glData
{
    void CheckErrors();

    std::shared_ptr<commandBuffer> CommandBuffer;

    framebufferHandle SwapchainFramebuffer;
};

}