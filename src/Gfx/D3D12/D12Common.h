#pragma once
#include <iostream>
#include <windows.h>

#define GET_CONTEXT(data, context) \
    std::shared_ptr<d3d12Data> data = std::static_pointer_cast<d3d12Data>(context->ApiContextData); \

namespace gfx
{
    inline void ThrowIfFailed(HRESULT hr)
    {
        if (FAILED(hr))
        {
            assert(false);
            std::cout << "Error " << std::endl;
        }
    }

    namespace d12Constants
    {
        static const u32 FrameCount = 2;
        static const u32 MaxResourceBindings = 1024; 
    }
}
