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

        std::vector<vk::WriteDescriptorSet> DescriptorWrites(Uniforms.size());
        std::vector<vk::DescriptorBufferInfo> DescriptorBuffers(Uniforms.size());
        for(sz i=0; i<Uniforms.size(); i++)
        {
            DescriptorWrites[i].setDstSet(VkUniformData->DescriptorInfos[PipelineHandle].DescriptorSet)
                                .setDstBinding(Binding.second)
                                .setDstArrayElement(0)
                                .setDescriptorCount(1);

            switch (Uniforms[i].Type)
            {
            case uniformType::Buffer :
                std::shared_ptr<buffer> Buffer = std::static_pointer_cast<buffer>(Uniforms[i].Resource);
                std::shared_ptr<vkBufferData> VKBuffer = std::static_pointer_cast<vkBufferData>(Buffer->ApiData);
                vk::DescriptorBufferInfo DescriptorBufferInfo(VKBuffer->Handle, Uniforms[i].Binding, Buffer->Size);

                DescriptorWrites[i].setDescriptorType(vk::DescriptorType::eUniformBuffer);
                DescriptorBuffers[i] = vk::DescriptorBufferInfo(VKBuffer->Handle, 0, Buffer->Size);
                DescriptorWrites[i].setPBufferInfo(&DescriptorBuffers[i]);
                break;  
            }
        }

        VkData->Device.updateDescriptorSets(DescriptorWrites, {});
    }

}

}