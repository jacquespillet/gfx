#pragma once
#include "Types.h"

namespace gfx
{
struct memoryStatistics
{
    sz _AllocatedBytes;
    sz _TotalBytes;
    u32 _AllocationCount;

    void Add(sz A);
};

class heapAllocator 
{
public:
    void Init(sz Size);
    void* Allocate(sz Size, sz Alignment);
    void Deallocate(void* Pointer);
    void Destroy();
private:
    void* _TlsfHandle;
    void* _Memory;

    sz _AllocatedSize;
    sz _MaxSize;
};

class stackAllocator
{
public:
    void InitMb(sz Size);

private:
    u8 *_Memory;
    sz _AllocatedSize;
    sz _TotalSize;
};

class memory
{
private:
    static memory* _Singleton;
    static sz _Size;
public:
    memory() = default;
    memory(memory &other) = delete;
    void operator=(const memory &) = delete;

    void Init();
    void Destroy();
    static memory *Get();

public:
    heapAllocator *GetSystemAllocator();
private:
    heapAllocator _SystemAllocator;
};

#define KiloBytes(Size) (Size * 1024 * 1024)
#define MegaBytes(Size) (Size * 1024 * 1024)
#define GigaBytes(Size) (Size * 1024 * 1024 * 1024)

sz MemoryAlign(sz Size, sz Alignment);

void* AllocateMemory(sz Size);

}