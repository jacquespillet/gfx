#pragma once
#include "Types.h"
#include <memory>
namespace gfx
{
struct buffer
{
    const char *Name;
    u32 Size;
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

}