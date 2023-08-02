#pragma once

#define GET_CONTEXT(data, context) \
    std::shared_ptr<d3d11Data> data = std::static_pointer_cast<d3d11Data>(context->ApiContextData); \

namespace gfx
{
inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        assert(false);
    }
}    
}