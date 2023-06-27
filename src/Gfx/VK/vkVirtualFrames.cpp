#include "../Include/GfxContext.h"
#include "../Include/Swapchain.h"
#include "vkVirtualFrames.h"
#include "VkGfxContext.h"
#include "vkMapping.h"
#include "VkSwapchain.h"
#include "VkImage.h"

namespace gfx
{
void virtualFrameProvider::Init(u64 FrameCount, u64 StageBufferSize)
{
    auto Context = context::Get();
    vkData *VkData = (vkData*)Context->ApiContextData;

    this->VirtualFrames.reserve(FrameCount);

    //Allocate X command buffers
    vk::CommandBufferAllocateInfo CommandBufferAllocateInfo;
    CommandBufferAllocateInfo.setCommandPool(VkData->CommandPool)
                             .setCommandBufferCount((u32)FrameCount)
                             .setLevel(vk::CommandBufferLevel::ePrimary);
    auto CommandBuffers = VkData->Device.allocateCommandBuffers(CommandBufferAllocateInfo);


    for(u64 i=0; i<FrameCount; i++)
    {
        //Add the virtuaFrame with 1 command buffer, 1 stageBuffer, 1 fence.
        auto Fence = VkData->Device.createFence(vk::FenceCreateInfo(vk::FenceCreateFlagBits::eSignaled));
        
        this->VirtualFrames.push_back({
            CreateVkCommandBuffer(CommandBuffers[i]),
            stageBuffer(StageBufferSize),  
            Fence
        });
    }    
}

virtualFrameProvider::virtualFrame &virtualFrameProvider::GetCurrentFrame()
{
    return this->VirtualFrames[this->CurrentFrame];
}


u32 virtualFrameProvider::GetPresentImageIndex()
{
    return this->PresentImageIndex;
}

void virtualFrameProvider::StartFrame()
{
    auto Context = context::Get();
    vkData *VkData = (vkData*)Context->ApiContextData;
    swapchain *Swapchain = Context->Swapchain;
    vkSwapchainData *VkSwapchainData = (vkSwapchainData*)Swapchain->ApiData;

    auto AcquireNextImage = VkData->Device.acquireNextImageKHR(VkSwapchainData->Handle, UINT64_MAX, VkData->ImageAvailableSemaphore);
    assert(AcquireNextImage.result == vk::Result::eSuccess || AcquireNextImage.result == vk::Result::eSuboptimalKHR);
    this->PresentImageIndex = AcquireNextImage.value;

    auto &Frame = this->GetCurrentFrame();

    vk::Result WaitFenceResult = VkData->Device.waitForFences(Frame.CommandQueueFence, false, UINT64_MAX);
    assert(WaitFenceResult == vk::Result::eSuccess);

    VkData->Device.resetFences(Frame.CommandQueueFence);

    this->IsFrameRunning=true;
    
    VkSwapchainData->CurrentIndex = this->PresentImageIndex;
}

void virtualFrameProvider::EndFrame()
{
    auto &Frame = this->GetCurrentFrame();

    auto Context = context::Get();
    vkData *VkData = (vkData*)Context->ApiContextData;
    swapchain *Swapchain = Context->Swapchain;
    vkSwapchainData *VkSwapchainData = (vkSwapchainData*)Swapchain->ApiData;


    auto LastPresentImageUsage = VkSwapchainData->SwapchainImageUsages[this->PresentImageIndex];
    image *PresentImage = VkSwapchainData->AcquireSwapchainImage(this->PresentImageIndex, imageUsage::UNKNOWN);
    vkImageData *VkImageData = (vkImageData*)PresentImage->ApiData;

    auto SubresourceRange = GetDefaultImageSubresourceRange(*PresentImage);

    vk::ImageMemoryBarrier PresentImageTransferDstToPresent;
    PresentImageTransferDstToPresent.setSrcAccessMask(ImageUsageToAccessFlags(LastPresentImageUsage))
                                    .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
                                    .setOldLayout(ImageUsageToImageLayout(LastPresentImageUsage))
                                    .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
                                    .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                                    .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                                    .setImage(VkImageData->Handle)
                                    .setSubresourceRange(SubresourceRange);

    vkCommandBufferData *VkCommandBufferData = (vkCommandBufferData*)Frame.Commands->ApiData;

    VkCommandBufferData->Handle.pipelineBarrier(
        ImageUsageToPipelineStage(LastPresentImageUsage),
        vk::PipelineStageFlagBits::eBottomOfPipe,
        {},
        {},
        {},
        PresentImageTransferDstToPresent
    );


    Frame.StagingBuffer.Flush();
    Frame.StagingBuffer.Reset();


    Frame.Commands->End();

    std::array WaitDstStageMask = {(vk::PipelineStageFlags)vk::PipelineStageFlagBits::eTransfer};
    
    vk::SubmitInfo SubmitInfo;
    SubmitInfo.setWaitSemaphores(VkData->ImageAvailableSemaphore)
                .setWaitDstStageMask(WaitDstStageMask)
                .setSignalSemaphores(VkData->RenderingFinishedSemaphore)
                .setCommandBuffers(VkCommandBufferData->Handle);
    
    VkData->DeviceQueue.submit(std::array{SubmitInfo}, Frame.CommandQueueFence);
}

void virtualFrameProvider::Present()
{
    
    auto Context = context::Get();
    vkData *VkData = (vkData*)Context->ApiContextData;
    swapchain *Swapchain = Context->Swapchain;
    vkSwapchainData *VkSwapchainData = (vkSwapchainData*)Swapchain->ApiData;

    vk::PresentInfoKHR PresentInfo;
    PresentInfo.setWaitSemaphores(VkData->RenderingFinishedSemaphore)
                .setSwapchains(VkSwapchainData->Handle)
                .setImageIndices(this->PresentImageIndex);

    auto PresentSuccess = VkData->DeviceQueue.presentKHR(PresentInfo);
    assert(PresentSuccess == vk::Result::eSuccess);

    this->CurrentFrame = (this->CurrentFrame + 1) % this->VirtualFrames.size();
    this->IsFrameRunning=false;    
}

}