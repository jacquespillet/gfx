#if API==VK
#include "VkBuffer.h"
#include "../Include/Buffer.h"
#include "VkMemoryAllocation.h"
#include "../Include/GfxContext.h"

namespace gfx
{

vk::Buffer GetBufferHandle(buffer *Buffer)
{
    vkBufferData *VkBufferData = (vkBufferData*)Buffer->ApiData;
    return VkBufferData->Handle;
}

void buffer::Init(size_t ByteSize, bufferUsage::value Usage, memoryUsage MemoryUsage)
{

    constexpr std::array BufferQueueFamilyIndices = {(u32)0};
    this->Destroy();

    //Set size, usage
    this->Size = ByteSize;
    vk::BufferCreateInfo BufferCreateInfo;
    BufferCreateInfo.setSize(Size)
                    .setUsage((vk::BufferUsageFlags)Usage)
                    .setSharingMode(vk::SharingMode::eExclusive)
                    .setQueueFamilyIndices(BufferQueueFamilyIndices);

    //Allocate with vma
    vkBufferData *VkBufferData = (vkBufferData*)this->ApiData;
    VkBufferData->Allocation = gfx::AllocateBuffer(BufferCreateInfo, MemoryUsage, &VkBufferData->Handle);    
}

u8 *buffer::MapMemory()
{
    vkBufferData *VkBufferData = (vkBufferData*)this->ApiData;

    //If unmapped, map it
    if(this->MappedData == nullptr)
    {
        this->MappedData = gfx::MapMemory(VkBufferData->Allocation);
    }
    return this->MappedData;    
}


void buffer::UnmapMemory()
{
    vkBufferData *VkBufferData = (vkBufferData*)this->ApiData;
    //Unmap and reset pointer
    gfx::UnmapMemory(VkBufferData->Allocation);
    this->MappedData=nullptr;
}


void buffer::FlushMemory(size_t ByteSize, size_t Offset)
{
    vkBufferData *VkBufferData = (vkBufferData*)this->ApiData;
    gfx::FlushMemory(VkBufferData->Allocation, ByteSize, Offset);
}


void buffer::Destroy()
{
    vkBufferData *VkBufferData = (vkBufferData*)this->ApiData;
    if(VkBufferData->Handle)
    {
        if(this->MappedData != nullptr)
        {
            this->UnmapMemory();
        }
        //Deallocate with vma
        DeallocateBuffer(VkBufferData->Handle, VkBufferData->Allocation);
        VkBufferData->Handle = vk::Buffer();
    }
}


void buffer::CopyData(const uint8_t *Data, size_t ByteSize, size_t Offset)
{
    assert(ByteSize + Offset <= this->Size);

    if(this->MappedData == nullptr)
    {
        (void)this->MapMemory();
        std::memcpy((void*)(this->MappedData + Offset), (const void*)Data, ByteSize);
        this->FlushMemory(ByteSize, Offset);
        this->UnmapMemory();
    }
    else
    {
        std::memcpy((void*)(this->MappedData + Offset), (const void*)Data, ByteSize);
    }    
}

stageBuffer::stageBuffer(){}
stageBuffer::stageBuffer(sz Size)
{
    Init(Size);
}
    

void stageBuffer::Init(sz Size)
{
    bufferHandle Handle = context::Get()->CreateBuffer(Size, bufferUsage::TransferSource, memoryUsage::CpuToGpu);    
    this->Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(Handle);
    this->CurrentOffset=0;
    this->Buffer->MapMemory();
}

stageBuffer::allocation stageBuffer::Submit(const uint8_t *Data, u32 ByteSize)
{
    assert(this->CurrentOffset + ByteSize <= this->Buffer->Size);

    if(Data != nullptr)
    {
        this->Buffer->CopyData(Data, ByteSize, this->CurrentOffset);
    }

    this->CurrentOffset += ByteSize;
    return allocation{ByteSize, this->CurrentOffset - ByteSize};
}


void stageBuffer::Flush()
{
    this->Buffer->FlushMemory(this->CurrentOffset, 0);
}

void stageBuffer::Reset()
{
    this->CurrentOffset=0;
}


}
#endif