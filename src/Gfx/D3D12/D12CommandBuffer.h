#pragma once
#include "../Include/Types.h"
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
struct framebuffer;
struct d3d12CommandBufferData
{
    ID3D12GraphicsCommandList *CommandList;
    ID3D12CommandAllocator *CommandAllocator;

    f32 ClearColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
    f32 ClearDepth = 1;
    u8 ClearStencil = 0;

    framebuffer *CurrentFramebuffer=nullptr;
};

commandBuffer *CreateD3D12CommandBuffer(ComPtr<ID3D12CommandAllocator> CommandAllocator);
}