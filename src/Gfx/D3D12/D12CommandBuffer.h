#pragma once
#include "../Include/Memory.h"
#include "../Include/GfxContext.h"
#include "../Include/CommandBuffer.h"
#include "D12Common.h"
#include "D12GfxContext.h"
#include <wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>

namespace gfx
{

struct d3d12CommandBufferData
{
    ID3D12GraphicsCommandList *CommandList;
    ID3D12CommandAllocator *CommandAllocator;
};

commandBuffer *CreateD3D12CommandBuffer(ComPtr<ID3D12CommandAllocator> CommandAllocator);
}