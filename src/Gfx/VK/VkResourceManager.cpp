#if GFX_API==GFX_VK
#include "../Include/Memory.h"
#include "VkResourceManager.h"
#include "VkPipeline.h"

namespace gfx
{
    void resourceManager::InitApiSpecific()
    {
        ApiData = std::make_shared<vkResourceManagerData>();
        std::shared_ptr<vkResourceManagerData> VkResourceManagerData = std::static_pointer_cast<vkResourceManagerData>(ApiData);
        
        VkResourceManagerData->DescriptorSetLayouts.Init(2048, sizeof(descriptorSetLayout));
    }
}

#endif