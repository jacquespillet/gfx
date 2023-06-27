#if GFX_API==GFX_D3D12
#include "../Include/Memory.h"
#include "../Include/GfxContext.h"

namespace gfx
{
    void resourceManager::InitApiSpecific()
    {
        // ApiData = (vkResourceManagerData*)AllocateMemory(sizeof(vkResourceManagerData)); 
        // vkResourceManagerData *VkResourceManagerData = (vkResourceManagerData*)ApiData;
        
        // VkResourceManagerData->DescriptorSetLayouts.Init(2048, sizeof(descriptorSetLayout));
    }
}

#endif