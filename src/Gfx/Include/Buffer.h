#pragma once
#include "Types.h"
#include "VertexInput.h"


#include <memory>
#include <unordered_map>
namespace gfx
{
struct buffer
{
    const char *Name;
    sz Size;
    sz Stride;
    u8 *MappedData = nullptr;
    
    std::shared_ptr<void> ApiData;

    memoryUsage MemoryUsage;
    void Init(size_t ByteSize, sz Stride, bufferUsage::value Usage, memoryUsage MemoryUsage);
    void CopyData(const uint8_t *Data, size_t ByteSize, size_t Offset);
    u8 *MapMemory();
    void UnmapMemory();
    void FlushMemory(size_t ByteSize, size_t Offset);
    void Destroy();
};

struct stageBuffer
{
    struct allocation
    {
        u32 Size;
        u32 Offset;
    };

    bufferHandle BufferHandle;
    u32 CurrentOffset;

    stageBuffer();
    stageBuffer(size_t ByteSize);
    void Init(size_t ByteSize);
    allocation Submit(const uint8_t *Data, u32 ByteSize);
    void Flush();
    void Reset();
    void Destroy();
    buffer *GetBuffer();
};

struct vertexStreamData
{

    void *Data=nullptr;
    sz Size=0;
    sz Stride=0;
    u32 StreamIndex=0;
    bufferHandle Buffer=0;
    u32 AttributesCount=0;
    vertexInputRate InputRate = vertexInputRate::PerVertex;
    vertexInputAttribute InputAttributes[commonConstants::MaxVertexAttributes];

    vertexStreamData &Reset();
    vertexStreamData &SetData(void *Data);
    vertexStreamData &SetSize(u32 Size);
    vertexStreamData &SetInputRate(vertexInputRate InputRate);
    vertexStreamData &SetStride(u32 Stride);
    vertexStreamData &SetStreamIndex(u32 StreamIndex);
    vertexStreamData &AddAttribute(vertexInputAttribute Attribute);
};

struct vertexBufferCreateInfo
{
    vertexStreamData VertexStreams[commonConstants::MaxVertexStreams];
    u32 NumVertexStreams=0;

    // TODO: Add a shaderDeviceAddress flag here (for now it automatically does when rtx is enabled, but that might not be efficient)

    vertexBufferCreateInfo &Init();
    vertexBufferCreateInfo &Reset();
    vertexBufferCreateInfo &AddVertexStream(vertexStreamData StreamData);
};

struct vertexBuffer
{
    vertexStreamData VertexStreams[commonConstants::MaxVertexStreams];
    u32 NumVertexStreams=0;

    std::shared_ptr<void> ApiData = nullptr;
};

}