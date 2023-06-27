#if GFX_API==GFX_VK
#include "../Include/Memory.h"
#include "VkResourceManager.h"
#include "VkPipeline.h"

namespace gfx
{
    void resourceManager::InitApiSpecific()
    {
        ApiData = (vkResourceManagerData*)AllocateMemory(sizeof(vkResourceManagerData)); 
        vkResourceManagerData *VkResourceManagerData = (vkResourceManagerData*)ApiData;
        
        VkResourceManagerData->DescriptorSetLayouts.Init(2048, sizeof(descriptorSetLayout));
    }
}

#endif