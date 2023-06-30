#pragma once
#include "../Include/Uniform.h"
#include "../Include/Types.h"
namespace gfx
{

struct vkUniformData
{
    vk::DescriptorSet DescriptorSet = VK_NULL_HANDLE;
    vk::DescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
    b8 Initialized=false;
};

}