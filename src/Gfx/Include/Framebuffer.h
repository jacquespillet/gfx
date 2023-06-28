#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>
#include <memory>
using namespace Microsoft::WRL;

namespace gfx
{
struct framebuffer
{
    u32 Width, Height;
    
    std::shared_ptr<void>  ApiData;
};  
}