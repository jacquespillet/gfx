#include "D12Buffer.h"
#include "../Include/Buffer.h"
#include "../Include/Context.h"
#include "D12Common.h"
#include "D12Context.h"
#include "D12CommandBuffer.h"
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


static sz CalcConstantBufferByteSize(sz byteSize)
{
    return (byteSize + 255) & ~255;
}

void buffer::Init(size_t ByteSize, sz Stride, bufferUsage::value Usage, memoryUsage MemoryUsage)
{
    this->Size = ByteSize;
    this->Stride = Stride;
    this->MemoryUsage = MemoryUsage;

    if(Usage == bufferUsage::UniformBuffer)
    {
        this->Size = CalcConstantBufferByteSize(ByteSize);
    }
    
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

    D3D12_RESOURCE_DESC BufferDesc = CD3DX12_RESOURCE_DESC::Buffer(this->Size);
    if(Usage == bufferUsage::StorageBuffer) BufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

    ThrowIfFailed(D12Data->Device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(HeapType),
        D3D12_HEAP_FLAG_NONE,
        &BufferDesc,
        InitialResourceState,
        nullptr,
        IID_PPV_ARGS(&D12BufferData->Handle)));
    
    D3D12_GPU_VIRTUAL_ADDRESS VA =  D12BufferData->Handle->GetGPUVirtualAddress();

    
    if(Usage == bufferUsage::UniformBuffer)
    {
        GET_CONTEXT(D12Data, context::Get());

        D12BufferData->OffsetInHeap = D12Data->CurrentHeapOffset;

        // Create a CBV descriptor
        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
        cbvDesc.BufferLocation = D12BufferData->Handle->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (u32)this->Size;
        D12Data->Device->CreateConstantBufferView(&cbvDesc, D12Data->GetCPUDescriptorAt(D12BufferData->OffsetInHeap));

        // Allocate descriptors by incrementing the handles
        D12Data->CurrentHeapOffset++;   
    }
    if(Usage == bufferUsage::StorageBuffer)
    {
        D12BufferData->OffsetInHeap = D12Data->CurrentHeapOffset;

        D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
        UAVDesc.Format = DXGI_FORMAT_UNKNOWN;
        UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        UAVDesc.Buffer.FirstElement = 0;
        UAVDesc.Buffer.NumElements = ByteSize / Stride; // Number of elements in the buffer
        UAVDesc.Buffer.StructureByteStride = Stride; // Size of each element in bytes
        UAVDesc.Buffer.CounterOffsetInBytes = 0;
        UAVDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
        D12Data->Device->CreateUnorderedAccessView(D12BufferData->Handle.Get(), nullptr, &UAVDesc, D12Data->GetCPUDescriptorAt(D12BufferData->OffsetInHeap));        
        
        D12Data->CurrentHeapOffset++;   
    }

    if(Usage == bufferUsage::IndexBuffer)
    {
        D12BufferData->IndexBufferView.BufferLocation = D12BufferData->Handle->GetGPUVirtualAddress();
        D12BufferData->IndexBufferView.SizeInBytes = (u32)ByteSize;    
        D12BufferData->IndexBufferView.Format = DXGI_FORMAT_R32_UINT;
    }
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
    GET_CONTEXT(D12Data, context::Get());

    assert(ByteSize + Offset <= this->Size);

    if(MemoryUsage == memoryUsage::GpuOnly)
    {
        auto StageBuffer = D12Data->StageBuffer;
        auto CommandBuffer = D12Data->ImmediateCommandBuffer;

        CommandBuffer->Begin();

        auto Allocation = StageBuffer.Submit((uint8_t*)Data, (u32)ByteSize);

        CommandBuffer->CopyBuffer(
            gfx::bufferInfo {StageBuffer.GetBuffer(), Allocation.Offset},
            gfx::bufferInfo {this, 0},
            Allocation.Size
        );  
        
        StageBuffer.Flush();
        CommandBuffer->End();

        context::Get()->SubmitCommandBufferImmediate(CommandBuffer.get());
        StageBuffer.Reset();    
    }
    else
    {
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
}


stageBuffer::stageBuffer(){}
stageBuffer::stageBuffer(sz Size)
{
    Init(Size);
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

void stageBuffer::Init(sz Size)
{
    BufferHandle = context::Get()->CreateBuffer(Size, bufferUsage::TransferSource, memoryUsage::CpuToGpu);    
    buffer *Buffer = context::Get()->GetBuffer(BufferHandle);
    this->CurrentOffset=0;
    Buffer->MapMemory();
}

buffer *stageBuffer::GetBuffer()
{
    buffer *Buffer = context::Get()->GetBuffer(BufferHandle);
    return Buffer;
}

void stageBuffer::Destroy()
{
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