#if GFX_API==GFX_VK
#include <memory>

#include "VkBuffer.h"
#include "../Include/Buffer.h"
#include "../Include/GfxContext.h"
#include "VkMemoryAllocation.h"
#include "VkGfxContext.h"

namespace gfx
{

vk::Buffer GetBufferHandle(buffer *Buffer)
{
    std::shared_ptr<vkBufferData> VkBufferData = std::static_pointer_cast<vkBufferData>(Buffer->ApiData);
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
    std::shared_ptr<vkBufferData> VkBufferData = std::static_pointer_cast<vkBufferData>(this->ApiData);
    VkBufferData->Allocation = gfx::AllocateBuffer(BufferCreateInfo, MemoryUsage, &VkBufferData->Handle);    
}

u8 *buffer::MapMemory()
{
    std::shared_ptr<vkBufferData> VkBufferData = std::static_pointer_cast<vkBufferData>(this->ApiData);

    //If unmapped, map it
    if(this->MappedData == nullptr)
    {
        this->MappedData = gfx::MapMemory(VkBufferData->Allocation);
    }
    return this->MappedData;    
}


void buffer::UnmapMemory()
{
    std::shared_ptr<vkBufferData> VkBufferData = std::static_pointer_cast<vkBufferData>(this->ApiData);
    //Unmap and reset pointer
    gfx::UnmapMemory(VkBufferData->Allocation);
    this->MappedData=nullptr;
}


void buffer::FlushMemory(size_t ByteSize, size_t Offset)
{
    std::shared_ptr<vkBufferData> VkBufferData = std::static_pointer_cast<vkBufferData>(this->ApiData);
    gfx::FlushMemory(VkBufferData->Allocation, ByteSize, Offset);
}


void buffer::Destroy()
{
    std::shared_ptr<vkBufferData> VkBufferData = std::static_pointer_cast<vkBufferData>(this->ApiData);
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
    buffer *Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(BufferHandle);
    return Buffer;
}

void stageBuffer::Init(sz Size)
{
    BufferHandle = context::Get()->CreateBuffer(Size, bufferUsage::TransferSource, memoryUsage::CpuToGpu);    
    buffer *Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(BufferHandle);
    this->CurrentOffset=0;
    Buffer->MapMemory();
}

stageBuffer::allocation stageBuffer::Submit(const uint8_t *Data, u32 ByteSize)
{
    buffer *Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(BufferHandle);
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
    buffer *Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(BufferHandle);
    Buffer->FlushMemory(this->CurrentOffset, 0);
}

void stageBuffer::Reset()
{
    this->CurrentOffset=0;
}

void stageBuffer::Destroy()
{
    buffer *Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(BufferHandle);
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

vertexBuffer &vertexBuffer::Init()
{
    Reset();
    return *this;
}

vertexBuffer &vertexBuffer::Reset()
{
    NumVertexStreams=0;
    for (sz i = 0; i < MaxVertexStreams; i++)
    {
        this->VertexStreams[i].StreamIndex = (u32)-1;
    }
    ApiData = nullptr;

    VertexBufferHandle = InvalidHandle;
    
    return *this;
}

vertexBuffer &vertexBuffer::AddVertexStream(vertexStreamData StreamData)
{
    this->VertexStreams[NumVertexStreams++] = StreamData;   
    return *this;
}

vertexBuffer &vertexBuffer::Create()
{
    GET_CONTEXT(VkData, context::Get());
    for(sz i=0; i<NumVertexStreams; i++)
    {
        std::vector<vertexInputAttribute> Attributes(VertexStreams[i].AttributesCount);
        memcpy(&Attributes[0], &VertexStreams[i].InputAttributes, VertexStreams[i].AttributesCount * sizeof(vertexInputAttribute));

        VertexStreams[i].Buffer = context::Get()->CreateVertexBuffer((f32*)VertexStreams[i].Data, VertexStreams[i].Size, VertexStreams[i].Stride, Attributes);
    }
    
    return *this;
}


}
#endif