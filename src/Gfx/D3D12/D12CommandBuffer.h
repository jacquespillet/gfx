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
    ComPtr<ID3D12GraphicsCommandList> CommandList;
    ComPtr<ID3D12CommandAllocator> CommandAllocator;

    framebuffer *CurrentFramebuffer=nullptr;

    pipeline *CurrentPipeline = nullptr;
};

std::shared_ptr<commandBuffer> CreateD3D12CommandBuffer(ComPtr<ID3D12CommandAllocator> CommandAllocator);
}