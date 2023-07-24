#if GFX_API==GFX_VK
#include <memory>

#include "VkBuffer.h"
#include "../Include/Buffer.h"
#include "../Include/Context.h"
#include "VkMemoryAllocation.h"
#include "VkContext.h"

namespace gfx
{

vk::Buffer GetBufferHandle(buffer *Buffer)
{
    GET_API_DATA(VkBufferData, vkBufferData, Buffer);
    return VkBufferData->Handle;
}

void buffer::Init(size_t ByteSize, sz Stride, bufferUsage::value Usage, memoryUsage MemoryUsage)
{
    constexpr std::array BufferQueueFamilyIndices = {(u32)0};
    this->Destroy();

    //We assume that if the buffer is index or vertex, we'll copy to it
    if(Usage & bufferUsage::IndexBuffer || Usage & bufferUsage::VertexBuffer)
        Usage |= bufferUsage::TransferDestination;
    
    if(Usage & bufferUsage::StorageBuffer && MemoryUsage == memoryUsage::GpuOnly)
        Usage |= bufferUsage::TransferDestination;

    //Set size, usage
    this->Size = ByteSize;
    this->Stride = Stride;
    this->MemoryUsage = MemoryUsage;

    vk::BufferCreateInfo BufferCreateInfo;
    BufferCreateInfo.setSize(Size)
                    .setUsage((vk::BufferUsageFlags)Usage)
                    .setSharingMode(vk::SharingMode::eExclusive)
                    .setQueueFamilyIndices(BufferQueueFamilyIndices);

    //Allocate with vma
    GET_API_DATA(VkBufferData, vkBufferData, this);
    VkBufferData->Allocation = gfx::AllocateBuffer(BufferCreateInfo, MemoryUsage, &VkBufferData->Handle);    
}

u8 *buffer::MapMemory()
{
    GET_API_DATA(VkBufferData, vkBufferData, this);

    //If unmapped, map it
    if(this->MappedData == nullptr)
    {
        this->MappedData = gfx::MapMemory(VkBufferData->Allocation);
    }
    return this->MappedData;    
}


void buffer::UnmapMemory()
{
    GET_API_DATA(VkBufferData, vkBufferData, this);
    //Unmap and reset pointer
    gfx::UnmapMemory(VkBufferData->Allocation);
    this->MappedData=nullptr;
}


void buffer::FlushMemory(size_t ByteSize, size_t Offset)
{
    GET_API_DATA(VkBufferData, vkBufferData, this);
    gfx::FlushMemory(VkBufferData->Allocation, ByteSize, Offset);
}


void buffer::Destroy()
{
    GET_API_DATA(VkBufferData, vkBufferData, this);
    if(VkBufferData && VkBufferData->Handle)
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
buffer *stageBuffer::GetBuffer()
{
    buffer *Buffer = context::Get()->GetBuffer(BufferHandle);
    return Buffer;
}

void stageBuffer::Init(sz Size)
{
    BufferHandle = context::Get()->CreateBuffer(Size, bufferUsage::TransferSource, memoryUsage::CpuToGpu);    
    buffer *Buffer = context::Get()->GetBuffer(BufferHandle);
    this->CurrentOffset=0;
    Buffer->MapMemory();
}

stageBuffer::allocation stageBuffer::Submit(const uint8_t *Data, u32 ByteSize)
{
    buffer *Buffer = context::Get()->GetBuffer(BufferHandle);
    assert(this->CurrentOffset + ByteSize <= Buffer->Size);

    if(Data != nullptr)
    {
        Buffer->CopyData(Data, ByteSize, this->CurrentOffset);
    }

    this->CurrentOffset += ByteSize;
    return allocation{ByteSize, this->CurrentOffset - ByteSize};
}


void stageBuffer::Flush()
{
    buffer *Buffer = context::Get()->GetBuffer(BufferHandle);
    Buffer->FlushMemory(this->CurrentOffset, 0);
}

void stageBuffer::Reset()
{
    this->CurrentOffset=0;
}

void stageBuffer::Destroy()
{
    buffer *Buffer = context::Get()->GetBuffer(BufferHandle);
    Buffer->Destroy();
    GET_CONTEXT(VkData, context::Get());
    context::Get()->ResourceManager.Buffers.ReleaseResource(BufferHandle);
}



vertexStreamData &vertexStreamData::Reset()
{
    Data=nullptr;
    Size=0;
    Stride=0;
    StreamIndex=0;
    Buffer=0;
    AttributesCount=0;
    return *this;
}

vertexStreamData &vertexStreamData::SetData(void *Data)
{
    this->Data = Data;
    return *this;
}

vertexStreamData &vertexStreamData::SetInputRate(vertexInputRate InputRate)
{
    this->InputRate = InputRate;
    return *this;
}

vertexStreamData &vertexStreamData::SetSize(u32 Size)
{
    this->Size = Size;
    return *this;
}
vertexStreamData &vertexStreamData::SetStride(u32 Stride)
{
    this->Stride = Stride;
    return *this;
}
vertexStreamData &vertexStreamData::SetStreamIndex(u32 StreamIndex)
{
    this->StreamIndex = StreamIndex;
    return *this;
}
vertexStreamData &vertexStreamData::AddAttribute(vertexInputAttribute Attribute)
{
    this->InputAttributes[this->AttributesCount++] = Attribute;
    return *this;
}

vertexBufferCreateInfo &vertexBufferCreateInfo::Init()
{
    Reset();
    return *this;
}

vertexBufferCreateInfo &vertexBufferCreateInfo::Reset()
{
    NumVertexStreams=0;
    for (sz i = 0; i < commonConstants::MaxVertexStreams; i++)
    {
        this->VertexStreams[i].StreamIndex = (u32)-1;
    }
    return *this;
}

vertexBufferCreateInfo &vertexBufferCreateInfo::AddVertexStream(vertexStreamData StreamData)
{
    this->VertexStreams[NumVertexStreams++] = StreamData;   
    return *this;
}


}
#endif