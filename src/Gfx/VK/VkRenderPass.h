#pragma once

#include "../Include/RenderPass.h"
#include <vulkan/vulkan.hpp>

namespace gfx
{
struct vkRenderPassData
{
    vk::RenderPass NativeHandle;
};
}