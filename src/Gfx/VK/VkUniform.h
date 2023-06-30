#pragma once
#include "../Include/Uniform.h"
#include "../Include/Types.h"
#include <unordered_map>
namespace gfx
{
struct descriptorInfo
{
    vk::DescriptorSet DescriptorSet = VK_NULL_HANDLE;
    vk::DescriptorSetLayout DescriptorSetLayout = VK_NULL_HANDLE;
};

struct vkUniformData
{
    std::unordered_map<pipelineHandle, descriptorInfo> DescriptorInfos;
    b8 Initialized=false;
};

}