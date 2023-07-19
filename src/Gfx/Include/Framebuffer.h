#pragma once
#include "Types.h"
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <memory>
#include <vector>

using namespace Microsoft::WRL;

namespace gfx
{

struct framebufferCreateInfo
{
    u32 Width = 1024;
    u32 Height = 1024;
    std::vector<format> ColorFormats;
    format DepthFormat;
    f32 ClearValues[4];
    f32 ClearDepth = 1;
    u8 ClearStencil = 0;

    framebufferCreateInfo &SetSize(u32 Width, u32 Height);
    framebufferCreateInfo &AddColorFormat(format Format);
    framebufferCreateInfo &SetDepthFormat(format Format);
    framebufferCreateInfo &SetClearColor(f32 R, f32 G, f32 B, f32 A);
    framebufferCreateInfo &SetClearDepthStencil(f32 Depth, u8 Stencil);
        
    static framebufferCreateInfo Default(u32 Width, u32 Height){
        framebufferCreateInfo Result = {};
        Result.SetSize(Width, Height)
              .AddColorFormat(format::R8G8B8A8_UNORM)
              .SetDepthFormat(format::D24_UNORM_S8_UINT);
        return Result;
    }
};

struct framebuffer
{
    u32 Width, Height;
    std::shared_ptr<void>  ApiData;
    renderPassHandle RenderPass;
};  
}