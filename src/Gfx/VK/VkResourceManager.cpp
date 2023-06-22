#if API==VK
#include "VkResourceManager.h"
#include "VkPipeline.h"

namespace gfx
{
    void resourceManager::InitApiSpecific()
    {
        ApiData = new vkResourceManagerData(); 
        vkResourceManagerData *VkResourceManagerData = (vkResourceManagerData*)ApiData;
        
        VkResourceManagerData->DescriptorSetLayouts.Init(2048, sizeof(descriptorSetLayout));
    }
}

#endif