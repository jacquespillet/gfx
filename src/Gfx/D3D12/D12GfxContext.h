#pragma once

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
};

}