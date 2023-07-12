#pragma once
#include "../Include/Types.h"
#include "../Include/Framebuffer.h"
#include "D12Framebuffer.h"
#include "D12Common.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
using namespace Microsoft::WRL;

namespace gfx
{
struct d3d12SwapchainData
{
    ComPtr<IDXGISwapChain3> SwapChain;    
    framebufferHandle FramebufferHandle;
    ComPtr<ID3D12Resource> Buffers[d12Constants::FrameCount] = {};


    u32 GetFrameIndex()
    {
        return this->FrameIndex;
    }
    void SetFrameIndex(u32 _FrameIndex)
    {
        this->FrameIndex = _FrameIndex;
        framebuffer *Framebuffer = (framebuffer*) context::Get()->ResourceManager.Framebuffers.GetResource(FramebufferHandle);
        std::shared_ptr<d3d12FramebufferData> D12FramebufferData = std::static_pointer_cast<d3d12FramebufferData>(Framebuffer->ApiData);
        D12FramebufferData->CurrentTarget = _FrameIndex;
    }
private:
    u32 FrameIndex;
};

}