#pragma once
#include "VkGfxContext.h"
namespace gfx
{
static const u8 MaxDescriptorSetLayouts = 8;


struct descriptorBinding
{
    vk::DescriptorType Type;
    u16 Start=0;
    u16 Count=0;
    u16 Set=0;

    const char *Name=nullptr;
};

struct descriptorSetLayout
{
    
    vk::DescriptorSetLayout NativeHandle;

    vk::DescriptorSetLayoutBinding *BindingNativeHandle=nullptr;
    descriptorBinding *Bindings=nullptr;
    u8 *IndexToBinding=nullptr;
    
    u16 BindingCount=0;
    u16 SetIndex=0;
    u8 Bindless=0;
    u8 Dynamic = 0;

    b8 UsedBindings[vkConstants::MaxDescriptorsPerSet];

    descriptorSetLayoutHandle Handle;
};

struct vkPipelineData
{
    shaderStateHandle ShaderState;
 
    pipelineHandle Handle;

    vk::Pipeline NativeHandle;
    vk::PipelineLayout PipelineLayout;
    vk::PipelineBindPoint BindPoint;

    //TODO: Descriptor sets
    descriptorSetLayout *DescriptorSetLayouts[MaxDescriptorSetLayouts];
    descriptorSetLayoutHandle DescriptorSetLayoutHandles[MaxDescriptorSetLayouts];

    u32 NumActiveLayouts = 0;
};

}

