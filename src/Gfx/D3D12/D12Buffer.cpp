#include "D12Buffer.h"
#include "../Include/Buffer.h"
#include "../Include/GfxContext.h"
#include "D12Common.h"
#include "D12GfxContext.h"
#include <d3dx12.h>

namespace gfx
{

void d3d12BufferData::Transition(ID3D12GraphicsCommandList *CommandList, D3D12_RESOURCE_STATES DestinationState)
{
    D3D12_RESOURCE_BARRIER barrierDesc = {};
    barrierDesc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrierDesc.Transition.pResource = this->Handle.Get();
    barrierDesc.Transition.StateBefore = this->ResourceState;
    barrierDesc.Transition.StateAfter = DestinationState;
    barrierDesc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    CommandList->ResourceBarrier(1, &barrierDesc);
    this->ResourceState = DestinationState;
}

void buffer::Init(size_t ByteSize, bufferUsage::value Usage, memoryUsage MemoryUsage)
{
    this->Size = ByteSize;
    
    ApiData = std::make_shared<d3d12BufferData>();
    std::shared_ptr<d3d12BufferData> D12BufferData = std::static_pointer_cast<d3d12BufferData>(ApiData);
    
    GET_CONTEXT(D12Data, context::Get());

    D3D12_RESOURCE_STATES InitialResourceState = (D3D12_RESOURCE_STATES)Usage;

    D3D12_HEAP_TYPE HeapType = D3D12_HEAP_TYPE_DEFAULT;
    if(MemoryUsage == memoryUsage::CpuToGpu)
    {
        HeapType = D3D12_HEAP_TYPE_UPLOAD;
        InitialResourceState = D3D12_RESOURCE_STATE_GENERIC_READ;
    }
    else if(MemoryUsage == memoryUsage::GpuToCpu)
    {
        HeapType = D3D12_HEAP_TYPE_READBACK;
        InitialResourceState = D3D12_RESOURCE_STATE_COPY_DEST;
    }
    
    D12BufferData->ResourceState = InitialResourceState;
    D12BufferData->HeapType = HeapType;

    ThrowIfFailed(D12Data->Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(HeapType),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(ByteSize),
        InitialResourceState,
        nullptr,
        IID_PPV_ARGS(&D12BufferData->Handle)));
}

u8 *buffer::MapMemory()
{
    std::shared_ptr<d3d12BufferData> D12BufferData = std::static_pointer_cast<d3d12BufferData>(this->ApiData);

    //If unmapped, map it
    if(this->MappedData == nullptr)
    {
        ThrowIfFailed(D12BufferData->Handle->Map(0, nullptr, reinterpret_cast<void**>(&this->MappedData)));
    }
    return this->MappedData;    
}


void buffer::UnmapMemory()
{
    std::shared_ptr<d3d12BufferData> D12BufferData = std::static_pointer_cast<d3d12BufferData>(this->ApiData);
    //Unmap and reset pointer
    D12BufferData->Handle->Unmap(0, nullptr);
    this->MappedData=nullptr;
}

void buffer::FlushMemory(size_t ByteSize, size_t Offset)
{
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

void stageBuffer::Init(sz Size)
{
    bufferHandle Handle = context::Get()->CreateBuffer(Size, bufferUsage::TransferSource, memoryUsage::CpuToGpu);    
    this->Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(Handle);
    this->CurrentOffset=0;
    this->Buffer->MapMemory();
}

}