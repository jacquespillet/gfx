#include "../Include/Uniform.h"
#include "../Include/Context.h"
#include "../Include/Framebuffer.h"
#include "../Include/AccelerationStructure.h"
#include "VkUniform.h"
#include "VkCommon.h"
#include "VkContext.h"
#include "vkImage.h"
#include "vkFramebuffer.h"
#include "VkBuffer.h"
#include "VkAccelerationStructure.h"

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
    GET_API_DATA(VkUniformData, vkUniformData, this);
    
    context::Get()->WaitIdle();
    for(auto &Binding : Bindings)
    {
        pipelineHandle PipelineHandle = Binding.first;
        pipeline *Pipeline = context::Get()->GetPipeline(PipelineHandle);

        std::vector<vk::WriteDescriptorSet> DescriptorWrites;
        std::vector<vk::DescriptorBufferInfo> DescriptorBuffers;
        std::vector<vk::DescriptorImageInfo> DescriptorImages;
        std::vector<vk::WriteDescriptorSetAccelerationStructureKHR> DescriptorAccelerationStructures;
        DescriptorWrites.reserve(Uniforms.size());
        DescriptorBuffers.reserve(Uniforms.size());
        DescriptorImages.reserve(Uniforms.size());
        for(sz i=0; i<Uniforms.size(); i++)
        {
            descriptorInfo &DescriptorInfo = VkUniformData->DescriptorInfos[Pipeline->Name];
            if(!VkUniformData->DescriptorInfos[Pipeline->Name].DescriptorSetLayout->UsedBindings[Uniforms[i].Binding]) continue;
            if(Uniforms[i].ResourceHandle == InvalidHandle) continue;

            vk::WriteDescriptorSet DescriptorWrite;
            DescriptorWrite.setDstSet(VkUniformData->DescriptorInfos[Pipeline->Name].DescriptorSet)
                                .setDstBinding(Uniforms[i].Binding)
                                .setDstArrayElement(0)
                                .setDescriptorCount(1);

            if(Uniforms[i].Type == uniformType::UniformBuffer)
            {
                buffer* Buffer = GetBuffer(i);
                GET_API_DATA(VkBuffer, vkBufferData, Buffer);
                
                DescriptorBuffers.push_back(vk::DescriptorBufferInfo(VkBuffer->Handle, 0, Buffer->Size));
                DescriptorWrite.setDescriptorType(vk::DescriptorType::eUniformBuffer)
                                   .setPBufferInfo(&DescriptorBuffers[DescriptorBuffers.size()-1]);
            }
            if(Uniforms[i].Type == uniformType::StorageBuffer)
            {
                buffer* Buffer = GetBuffer(i);
                GET_API_DATA(VkBuffer, vkBufferData, Buffer);
                
                DescriptorBuffers.push_back(vk::DescriptorBufferInfo(VkBuffer->Handle, 0, Buffer->Size));
                DescriptorWrite.setDescriptorType(vk::DescriptorType::eStorageBuffer)
                                   .setPBufferInfo(&DescriptorBuffers[DescriptorBuffers.size()-1]);
            }
            if(Uniforms[i].Type == uniformType::StorageImage)
            {
                image* Image = GetTexture(i);
                GET_API_DATA(VkImage, vkImageData, Image);

                DescriptorImages.push_back(vk::DescriptorImageInfo(VkImage->Sampler, VkImage->DefaultImageViews.NativeView, vk::ImageLayout::eGeneral));
                DescriptorWrite.setDescriptorType(vk::DescriptorType::eStorageImage)
                                   .setPImageInfo(&DescriptorImages[DescriptorImages.size()-1]);
            }
            else if(Uniforms[i].Type == uniformType::Texture2d) 
            {
                image* Image = GetTexture(i);
                GET_API_DATA(VKImage, vkImageData, Image);
                    // vk::DescriptorImageInfo DescriptorImageInfo(Texture->GetSampler(), Texture->GetNativeView(imageView::NATIVE), vk::ImageLayout::eShaderReadOnlyOptimal);

                DescriptorImages.push_back(vk::DescriptorImageInfo(VKImage->Sampler, VKImage->DefaultImageViews.NativeView, vk::ImageLayout::eShaderReadOnlyOptimal));
                DescriptorWrite.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                                   .setPImageInfo(&DescriptorImages[DescriptorImages.size()-1]);    
            }
            else if(Uniforms[i].Type == uniformType::AccelerationStructure)
            {
                accelerationStructure* AccelerationStructure = GetAccelerationStructure(i);
                GET_API_DATA(VkAccelerationStructure, vkAccelerationStructureData, AccelerationStructure);
                    // vk::DescriptorImageInfo DescriptorImageInfo(Texture->GetSampler(), Texture->GetNativeView(imageView::NATIVE), vk::ImageLayout::eShaderReadOnlyOptimal);

                DescriptorAccelerationStructures.emplace_back();
                vk::WriteDescriptorSetAccelerationStructureKHR &AccelerationStructureInfo = DescriptorAccelerationStructures.back();
                AccelerationStructureInfo.setAccelerationStructureCount(1).setPAccelerationStructures(&VkAccelerationStructure->AccelerationStructure);

                DescriptorWrite.setDescriptorType(vk::DescriptorType::eAccelerationStructureKHR)
                               .setPNext(&AccelerationStructureInfo);
            }
            else if(Uniforms[i].Type == uniformType::FramebufferRenderTarget)
            {
                framebuffer* Framebuffer = GetFramebuffer(i);
                GET_API_DATA(VKFramebuffer, vkFramebufferData, Framebuffer);
                // vk::DescriptorImageInfo DescriptorImageInfo(Textur   e->GetSampler(), Texture->GetNativeView(imageView::NATIVE), vk::ImageLayout::eShaderReadOnlyOptimal);

                std::shared_ptr<vkImageData> VKImage;
                if(Uniforms[i].ResourceIndex == uniformGroup::DepthAttachment)
                {
                    VKImage = std::static_pointer_cast<vkImageData>(VKFramebuffer->DepthStencilImage->ApiData);
                    DescriptorImages.push_back(vk::DescriptorImageInfo(VKImage->Sampler, VKImage->DefaultImageViews.DepthOnlyView, vk::ImageLayout::eDepthStencilReadOnlyOptimal));
                }
                else
                {
                    VKImage = std::static_pointer_cast<vkImageData>(VKFramebuffer->ColorImages[Uniforms[i].ResourceIndex]->ApiData);
                    DescriptorImages.push_back(vk::DescriptorImageInfo(VKImage->Sampler, VKImage->DefaultImageViews.NativeView, vk::ImageLayout::eShaderReadOnlyOptimal));
                }
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