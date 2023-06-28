#pragma once
#include "vkCommandBuffer.h"
#include "VkBuffer.h"
#include "../Include/Buffer.h"

#include <vulkan/vulkan.hpp>
#include <memory>

namespace gfx
{
class virtualFrameProvider
{
private:
    struct virtualFrame
    {
        std::shared_ptr<commandBuffer> Commands;
        stageBuffer StagingBuffer;
        vk::Fence CommandQueueFence;
    };
public:
    void Init(u64 FrameCount, u64 StageBufferSize);
    virtualFrame &GetCurrentFrame();
    u32 GetPresentImageIndex();

    void StartFrame();
    void EndFrame();
    void Present();

    void Destroy();
    
    std::vector<virtualFrame> VirtualFrames;

    u64 CurrentFrame=0;
    u32 PresentImageIndex=0;
    b8 IsFrameRunning=false;
};    
}