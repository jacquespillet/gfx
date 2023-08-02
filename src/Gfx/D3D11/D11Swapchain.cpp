#include "../Include/Swapchain.h"
#include "../Include/Types.h"
#include "D11Swapchain.h"
namespace gfx
{
void swapchain::Present()
{
    GET_API_DATA(D11Swapchain, d3d11Swapchain, this);
    D11Swapchain->Handle->Present(1, 0);
}
}