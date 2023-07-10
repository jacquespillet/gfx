#include "../Include/Uniform.h"
#include "../Include/GfxContext.h"
#include "VkUniform.h"
#include "VkCommon.h"
#include "VkGfxContext.h"

namespace gfx
{
void uniformGroup::Initialize()
{
    this->ApiData = std::make_shared<vkUniformData>();
}

void uniformGroup::Update()
{
    GET_CONTEXT(VkData, context::Get());
    std::shared_ptr<vkUniformData> VkUniformData = std::static_pointer_cast<vkUniformData>(this->ApiData);

    

    for(auto &Binding : Bindings)
    {
        pipelineHandle PipelineHandle = Binding.first;

        std::vector<vk::WriteDescriptorSet> DescriptorWrites;
        std::vector<vk::DescriptorBufferInfo> DescriptorBuffers;
        DescriptorWrites.reserve(Uniforms.size());
        DescriptorBuffers.reserve(Uniforms.size());
        for(sz i=0; i<Uniforms.size(); i++)
        {
            descriptorInfo &DescriptorInfo = VkUniformData->DescriptorInfos[PipelineHandle];
            if(!VkUniformData->DescriptorInfos[PipelineHandle].DescriptorSetLayout->UsedBindings[Uniforms[i].Binding]) continue;
            
            vk::WriteDescriptorSet DescriptorWrite;
            DescriptorWrite.setDstSet(VkUniformData->DescriptorInfos[PipelineHandle].DescriptorSet)
                                .setDstBinding(Uniforms[i].Binding)
                                .setDstArrayElement(0)
                                .setDescriptorCount(1);

            switch (Uniforms[i].Type)
            {
            case uniformType::Buffer :
                std::shared_ptr<buffer> Buffer = std::static_pointer_cast<buffer>(Uniforms[i].Resource);
                std::shared_ptr<vkBufferData> VKBuffer = std::static_pointer_cast<vkBufferData>(Buffer->ApiData);
                
                DescriptorBuffers.push_back(vk::DescriptorBufferInfo(VKBuffer->Handle, 0, Buffer->Size));
                DescriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer)
                                   .setPBufferInfo(&DescriptorBuffers[DescriptorBuffers.size()-1]);
                break;  
            }

            DescriptorWrites.push_back(DescriptorWrite);
        }

        VkData->Device.updateDescriptorSets(DescriptorWrites, {});
    }

}

}