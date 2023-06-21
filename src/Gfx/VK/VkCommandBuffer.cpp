#if API==VK
#include "VkCommandBuffer.h"
#include "../Include/Buffer.h"
#include "VkBuffer.h"
namespace gfx
{

commandBuffer *CreateVkCommandBuffer(vk::CommandBuffer VkCommandBuffer)
{
    commandBuffer *CommandBuffer = new commandBuffer();
    CommandBuffer->ApiData = new vkCommandBufferData();
    vkCommandBufferData *VkData = (vkCommandBufferData*)CommandBuffer->ApiData;
    VkData->Handle = VkCommandBuffer;

    return CommandBuffer;
}

void commandBuffer::Begin()
{
    vkCommandBufferData *VkCommandBufferData = (vkCommandBufferData*)this->ApiData;
    
    vk::CommandBufferBeginInfo CommandBufferBeginInfo;
    CommandBufferBeginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
    VkCommandBufferData->Handle.begin(CommandBufferBeginInfo);
}

void commandBuffer::End()
{
    vkCommandBufferData *VkCommandBufferData = (vkCommandBufferData*)this->ApiData;
    VkCommandBufferData->Handle.end();
}

void commandBuffer::CopyBuffer(const bufferInfo &Source, const bufferInfo &Destination, size_t ByteSize)
{
    vkCommandBufferData *VkCommandBufferData = (vkCommandBufferData*)this->ApiData;
    
    assert(Source.Resource->Size >= Source.Offset + ByteSize);
    assert(Destination.Resource->Size >= Destination.Offset + ByteSize);

    vk::BufferCopy BufferCopyInfo;
    BufferCopyInfo.setDstOffset(Destination.Offset)
                  .setSize(ByteSize)
                  .setSrcOffset(Source.Offset);
    
    vk::Buffer SourceHandle = GetBufferHandle(Source.Resource);
    vk::Buffer DestHandle = GetBufferHandle(Destination.Resource);

    VkCommandBufferData->Handle.copyBuffer(SourceHandle, DestHandle, BufferCopyInfo);
}

}

#endif