#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <memory>
using namespace Microsoft::WRL;

namespace gfx
{

struct framebufferCreateInfo
{
    u32 Width = 1024;
    u32 Height = 1024;
    std::vector<format> ColorFormats = { format::R8G8B8A8_UNORM};
    format DepthFormat = format::D24_UNORM_S8_UINT;
    static framebufferCreateInfo Default(u32 Width, u32 Height){
        return
        {
            Width, Height,
            {format::R8G8B8A8_UNORM},
			format::D24_UNORM_S8_UINT
        };
    }
};

struct framebuffer
{
    u32 Width, Height;
    std::shared_ptr<void>  ApiData;
    renderPassHandle RenderPass;
};  
}