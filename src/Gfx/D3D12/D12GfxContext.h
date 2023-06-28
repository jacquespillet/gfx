#pragma once
#include "../Include/Buffer.h"

#include <wrl.h>
using namespace Microsoft::WRL;
#include <d3d12.h>
#include <dxgi1_6.h>
#include "D12VirtualFrames.h"

namespace gfx
{

struct d3d12Data
{
    ComPtr<IDXGIFactory4> Factory;
    ComPtr<ID3D12Device> Device;
    ComPtr<ID3D12CommandQueue> CommandQueue;
    virtualFramesProvider VirtualFrames;

    std::shared_ptr<commandBuffer> ImmediateCommandBuffer;
    ComPtr<ID3D12CommandAllocator> ImmediateCommandAllocator;
    
    stageBuffer StageBuffer;
};

}