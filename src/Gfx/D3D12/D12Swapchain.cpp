#include "../Include/Context.h"
#include "../Include/Framebuffer.h"
#include "D12Swapchain.h"
#include "D12Framebuffer.h"
namespace gfx
{

u32 d3d12SwapchainData::GetFrameIndex()
{
    return this->FrameIndex;
}
void d3d12SwapchainData::SetFrameIndex(u32 _FrameIndex)
{
    this->FrameIndex = _FrameIndex;
    framebuffer *Framebuffer = context::Get()->GetFramebuffer(FramebufferHandle);
    std::shared_ptr<d3d12FramebufferData> D12FramebufferData = std::static_pointer_cast<d3d12FramebufferData>(Framebuffer->ApiData);
    D12FramebufferData->CurrentTarget = _FrameIndex;
}    
}