#pragma once
#include "Types.h"

namespace gfx
{
struct buffer
{
    const char *Name;
    u32 Size;
    u8 *MappedData;
    
    void *ApiData;

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

    buffer *Buffer;
    u32 CurrentOffset;

    void Init(size_t ByteSize);
    allocation Submit(const uint8_t *Data, u32 ByteSize);
    void Flush();
    void Reset();
};

}