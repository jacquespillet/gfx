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
    u8 *MappedData;
    
    std::shared_ptr<void> ApiData;

    void Init(size_t ByteSize, bufferUsage::value Usage, memoryUsage MemoryUsage);
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
    vertexInputAttribute InputAttributes[commonConstants::MaxVertexAttributes];

    vertexStreamData &Reset();
    vertexStreamData &SetData(void *Data);
    vertexStreamData &SetSize(u32 Size);
    vertexStreamData &SetStride(u32 Stride);
    vertexStreamData &SetStreamIndex(u32 StreamIndex);
    vertexStreamData &AddAttribute(vertexInputAttribute Attribute);
};

struct vertexBuffer
{
    vertexStreamData VertexStreams[commonConstants::MaxVertexStreams];
    u32 NumVertexStreams=0;

    std::shared_ptr<void> ApiData = nullptr;

    vertexBuffer &Init();
    vertexBuffer &Reset();
    vertexBuffer &AddVertexStream(vertexStreamData StreamData);
    vertexBuffer &Create();
};

}