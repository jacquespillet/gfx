#pragma once
#include "../Include/Types.h"
#define GET_CONTEXT(data, context) \
    std::shared_ptr<vkData> data = std::static_pointer_cast<vkData>(context->ApiContextData); \

namespace gfx
{
namespace vkConstants
{
    static const u8 MaxDescriptorsPerSet = 16;
    static const u8 MaxDescriptorSetLayouts = 8;
    static const s32 MaxSwapchainImages = 16;
}
}