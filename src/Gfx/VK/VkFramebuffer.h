#pragma once
#include <vulkan/vulkan.hpp>

namespace gfx
{
struct vkFramebufferData
{
    vk::Framebuffer Handle;

    std::shared_ptr<image> *ColorImages;
    std::shared_ptr<image> DepthStencilImage;
    
};
}