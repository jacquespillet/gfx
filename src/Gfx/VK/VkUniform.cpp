#include "../Include/Uniform.h"
#include "../Include/GfxContext.h"
#include "../Include/Framebuffer.h"
#include "VkUniform.h"
#include "VkCommon.h"
#include "VkGfxContext.h"
#include "vkImage.h"
#include "vkFramebuffer.h"

namespace gfx
{
uniformGroup &uniformGroup::Reset()
{
    this->ApiData = std::make_shared<vkUniformData>();
    return *this;
}

uniformGroup & uniformGroup::Update()
{
    GET_CONTEXT(VkData, context::Get());
    std::shared_ptr<vkUniformData> VkUniformData = std::static_pointer_cast<vkUniformData>(this->ApiData);

    

    for(auto &Binding : Bindings)
    {
        pipelineHandle PipelineHandle = Binding.first;

        std::vector<vk::WriteDescriptorSet> DescriptorWrites;
        std::vector<vk::DescriptorBufferInfo> DescriptorBuffers;
        std::vector<vk::DescriptorImageInfo> DescriptorImages;
        DescriptorWrites.reserve(Uniforms.size());
        DescriptorBuffers.reserve(Uniforms.size());
        DescriptorImages.reserve(Uniforms.size());
        for(sz i=0; i<Uniforms.size(); i++)
        {
            descriptorInfo &DescriptorInfo = VkUniformData->DescriptorInfos[PipelineHandle];
            if(!VkUniformData->DescriptorInfos[PipelineHandle].DescriptorSetLayout->UsedBindings[Uniforms[i].Binding]) continue;
            
            vk::WriteDescriptorSet DescriptorWrite;
            DescriptorWrite.setDstSet(VkUniformData->DescriptorInfos[PipelineHandle].DescriptorSet)
                                .setDstBinding(Uniforms[i].Binding)
                                .setDstArrayElement(0)
                                .setDescriptorCount(1);

            if(Uniforms[i].Type == uniformType::UniformBuffer)
            {
                buffer* Buffer = GetBuffer(i);
                std::shared_ptr<vkBufferData> VKBuffer = std::static_pointer_cast<vkBufferData>(Buffer->ApiData);
                
                DescriptorBuffers.push_back(vk::DescriptorBufferInfo(VKBuffer->Handle, 0, Buffer->Size));
                DescriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer)
                                   .setPBufferInfo(&DescriptorBuffers[DescriptorBuffers.size()-1]);
            }
            if(Uniforms[i].Type == uniformType::StorageBuffer)
            {
                buffer* Buffer = GetBuffer(i);
                std::shared_ptr<vkBufferData> VKBuffer = std::static_pointer_cast<vkBufferData>(Buffer->ApiData);
                
                DescriptorBuffers.push_back(vk::DescriptorBufferInfo(VKBuffer->Handle, 0, Buffer->Size));
                DescriptorWrite.setDescriptorType(vk::DescriptorType::eStorageBuffer)
                                   .setPBufferInfo(&DescriptorBuffers[DescriptorBuffers.size()-1]);
            }
            else if(Uniforms[i].Type == uniformType::Texture2d)
            {
                image* Image = GetTexture(i);
                std::shared_ptr<vkImageData> VKImage = std::static_pointer_cast<vkImageData>(Image->ApiData);
                    // vk::DescriptorImageInfo DescriptorImageInfo(Texture->GetSampler(), Texture->GetNativeView(imageView::NATIVE), vk::ImageLayout::eShaderReadOnlyOptimal);

                DescriptorImages.push_back(vk::DescriptorImageInfo(VKImage->Sampler, VKImage->DefaultImageViews.NativeView, vk::ImageLayout::eShaderReadOnlyOptimal));
                DescriptorWrite.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                                   .setPImageInfo(&DescriptorImages[DescriptorImages.size()-1]);
            }
            else if(Uniforms[i].Type == uniformType::FramebufferRenderTarget)
            {
                framebuffer* Framebuffer = GetFramebuffer(i);
                std::shared_ptr<vkFramebufferData> VKFramebuffer = std::static_pointer_cast<vkFramebufferData>(Framebuffer->ApiData);
                // vk::DescriptorImageInfo DescriptorImageInfo(Textur   e->GetSampler(), Texture->GetNativeView(imageView::NATIVE), vk::ImageLayout::eShaderReadOnlyOptimal);

                std::shared_ptr<vkImageData> VKImage = std::static_pointer_cast<vkImageData>(VKFramebuffer->ColorImages[Uniforms[i].ResourceIndex]->ApiData);
                DescriptorImages.push_back(vk::DescriptorImageInfo(VKImage->Sampler, VKImage->DefaultImageViews.NativeView, vk::ImageLayout::eShaderReadOnlyOptimal));
                DescriptorWrite.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                                   .setPImageInfo(&DescriptorImages[DescriptorImages.size()-1]);
            }

            DescriptorWrites.push_back(DescriptorWrite);
        }

        VkData->Device.updateDescriptorSets(DescriptorWrites, {});
    }

    return *this;
}

}