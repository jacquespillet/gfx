#pragma once
#include "Types.h"
#include <unordered_map>
#include "../Include/Memory.h"
#include <assert.h>
#include <memory>
namespace gfx
{
struct buffer;


class resourcePool
{
public:
    void Init(u32 PoolSize, u32 ResourceSize)
    {
        this->_PoolSize = PoolSize;
        this->_ResourceSize = ResourceSize;

        //Allocate the memory
        sz AllocationSize = PoolSize * (ResourceSize + sizeof(u32));
        _Memory = (u8*)AllocateMemory(AllocationSize);
        memset(_Memory, 0, AllocationSize);

        //
        _FreeIndices = (u32*)(_Memory + PoolSize * ResourceSize);
        _FreeIndicesHead=0;

        for(u32 i=0; i<PoolSize; i++)
        {
            _FreeIndices[i] = i;
        }

        _UsedIndices=0;
    }
    
    u32 ObtainResource()
    {
        if(_FreeIndicesHead < _PoolSize)
        {
            const u32 FreeIndex = _FreeIndices[_FreeIndicesHead++];
            ++_UsedIndices;
            return FreeIndex;
        }

        assert(false);
        return InvalidHandle;
    }

    void* GetResource(u32 Handle)
    {
        if(Handle != InvalidHandle)
        {
            return &_Memory[Handle * _ResourceSize];
        }
        return nullptr;
    }

    void ReleaseResource(u32 Handle)
    {
        _FreeIndices[--_FreeIndicesHead] = Handle;
        --_UsedIndices;
    }

    void FreeAllResources()
    {
        _FreeIndicesHead=0;
        _UsedIndices=0;
        for(u32 i=0; i<_PoolSize; i++)
        {
            _FreeIndices[i] = i;
        }
    }

    u32 GetPoolSize()
    {
        return _PoolSize;
    }

    void Destroy()
    {
        if(_FreeIndicesHead != 0)
        {
            printf("Resource pool has unfreed resources \n");
            for(sz i=0; i<_FreeIndicesHead; ++i)
            {
                printf("Resource %u\n", _FreeIndices[i]);
            }
        }

        assert(_UsedIndices==0);

        DeallocateMemory(_Memory);
    }

    
    u32 _UsedIndices=0;
protected:
    u32 _PoolSize=16;
    u32 _ResourceSize = 4;

    u8 *_Memory = nullptr;
    u32 *_FreeIndices = nullptr;

    u32 _FreeIndicesHead=0;
};

struct resourceManager
{
    void Init();
    void InitApiSpecific();
    void Destroy();
    void DestroyApiSpecific();
    resourcePool Buffers;
    resourcePool VertexBuffers;
    resourcePool Images;
    resourcePool Pipelines;
    resourcePool Shaders;
    resourcePool RenderPasses;
    resourcePool Framebuffers;

    std::shared_ptr<void> ApiData;
};
}