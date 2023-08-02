#include "../Include/Buffer.h"
#include "../Include/Context.h"
#include "D11Context.h"
#include "D11Buffer.h"
#include "D11Common.h"

#include <d3d11_1.h>
namespace gfx
{
static sz CalcConstantBufferByteSize(sz byteSize)
{
    return (byteSize + 255) & ~255;
}

void buffer::Init(size_t ByteSize, sz Stride, bufferUsage::value Usage, memoryUsage MemoryUsage)
{
    GET_CONTEXT(D11Data, context::Get());
    GET_API_DATA(D11Buffer, d3d11Buffer, this); 
    
    this->Size = ByteSize;
    this->Stride = Stride;
    this->MemoryUsage = MemoryUsage;
    
    if(Usage ==bufferUsage::UniformBuffer)
        this->Size = CalcConstantBufferByteSize(ByteSize);
    

    D3D11_BUFFER_DESC constantBufferDesc = {};
    constantBufferDesc.ByteWidth      = this->Size;
    constantBufferDesc.Usage          = D3D11_USAGE_DYNAMIC;
    constantBufferDesc.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    D11Data->Device->CreateBuffer(&constantBufferDesc, nullptr, &D11Buffer->Handle);   
}


u8 *buffer::MapMemory()
{
    GET_CONTEXT(D11Data, context::Get());
    GET_API_DATA(D11Buffer, d3d11Buffer, this); 

    //If unmapped, map it
    if(this->MappedData == nullptr)
    {
        D11Data->DeviceContext->Map(D11Buffer->Handle, 0, D3D11_MAP_WRITE_DISCARD, 0, &D11Buffer->MappedSubresource);
        this->MappedData = (u8*)D11Buffer->MappedSubresource.pData;
    }
    if(this->MappedData == nullptr) assert(false);
    return this->MappedData;    
}

void buffer::UnmapMemory()
{
    GET_CONTEXT(D11Data, context::Get());
    GET_API_DATA(D11Buffer, d3d11Buffer, this); 
    D11Data->DeviceContext->Unmap(D11Buffer->Handle, 0);
    this->MappedData=nullptr;
}

void buffer::CopyData(const uint8_t *Data, size_t ByteSize, size_t Offset)
{
    GET_CONTEXT(D11Data, context::Get());
    GET_API_DATA(D11Buffer, d3d11Buffer, this); 

    if(MemoryUsage == memoryUsage::GpuOnly)
    {
        assert(false);
    }
    else
    {
        if(this->MappedData == nullptr)
        {
            this->MappedData = MapMemory();
            memcpy(this->MappedData + Offset, Data, ByteSize);
            UnmapMemory();
        }
        else
        {
            memcpy(this->MappedData + Offset, Data, ByteSize);
        }
    }
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