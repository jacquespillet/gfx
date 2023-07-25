#pragma once
#include "../Include/Uniform.h"
#include "../Include/Types.h"
#include "VkPipeline.h"
#include <unordered_map>
namespace gfx
{
struct descriptorInfo
{
    vk::DescriptorSet DescriptorSet = VK_NULL_HANDLE;
    descriptorSetLayout *DescriptorSetLayout;
};

struct vkUniformData
{
    std::unordered_map<std::string, descriptorInfo> DescriptorInfos;
    b8 Initialized=false;
};

}