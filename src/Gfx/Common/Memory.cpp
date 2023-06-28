#include "../Include/Memory.h"
#include "tlsf.h"
#include <stdio.h>
#include <cstdlib>
#include <assert.h>

namespace gfx
{
void memoryStatistics::Add(sz A)
{
    if(A)
    {
        _AllocatedBytes += A;
        ++_AllocationCount;
    }
}

void heapAllocator::Init(sz Size)
{
    this->_Memory = malloc(Size);
    this->_MaxSize = Size;
    this->_AllocatedSize = 0;

    this->_TlsfHandle = tlsf_create_with_pool(this->_Memory, this->_MaxSize);

    printf("Created Heap Allocator of size %llu \n", this->_MaxSize);
}

void* heapAllocator::Allocate(sz Size, sz Alignment)
{
    void* AllocatedMemory = (Alignment==1) ? tlsf_malloc(_TlsfHandle, Size) : tlsf_memalign(_TlsfHandle, Alignment, Size);
    sz ActualSize = tlsf_block_size(AllocatedMemory);
    _AllocatedSize += ActualSize;

    return AllocatedMemory; 
}

void heapAllocator::Deallocate(void* Pointer)
{
    sz ActualSize = tlsf_block_size(Pointer);
    _AllocatedSize -= ActualSize;
    tlsf_free(_TlsfHandle, Pointer);
}

void ExitWalker(void* Ptr, sz Size, s32 Used, void* User)
{
    memoryStatistics *Stats = (memoryStatistics*)User;
    Stats->Add(Used ? Size : 0);

    if(Used)
    {
        printf("Found Active allocation %p, %llu\n", Ptr, Size);
    }
}

void heapAllocator::Destroy()
{
    memoryStatistics Stats{0, _MaxSize};
    pool_t Pool = tlsf_get_pool(_TlsfHandle);
    tlsf_walk_pool(Pool, ExitWalker, (void*)&Stats);

    if(Stats._AllocatedBytes)
    {
        printf("ERROR:Unfreed Memory Detected\n");
    }

    assert(Stats._AllocatedBytes == 0);

    tlsf_destroy(_TlsfHandle);
    free(_Memory);

}

void stackAllocator::InitMb(sz Size)
{
    sz SizeMb = MegaBytes(Size);

    _Memory = (u8*) malloc(SizeMb);
    _AllocatedSize = 0;
    _TotalSize = Size;

}

memory* memory::_Singleton= nullptr;
sz memory::_Size = MegaBytes(32) + tlsf_size() + 8;

memory *memory::Get()
{
    if(_Singleton==nullptr) _Singleton = new memory();
    return _Singleton;
}

void memory::Init()
{
    _SystemAllocator.Init(GigaBytes(2ull));
}

void memory::Destroy()
{
    _SystemAllocator.Destroy();
}

heapAllocator *memory::GetSystemAllocator()
{
    return &this->_SystemAllocator;
}

sz MemoryAlign(sz Size, sz Alignment)
{
    const sz AlignmentMask = Alignment -1;
    return (Size + AlignmentMask) & ~AlignmentMask;
}

void* AllocateMemory(sz Size)
{
    return memory::Get()->GetSystemAllocator()->Allocate(Size, 1);
}

void DeallocateMemory(void *Ptr)
{
    memory::Get()->GetSystemAllocator()->Deallocate(Ptr);
}

}