#include "../Include/Types.h"
#include "../Include/Framebuffer.h"

namespace gfx
{
framebufferCreateInfo& framebufferCreateInfo::SetSize(u32 Width, u32 Height)
{
    this->Width = Width;
    this->Height = Height;
    return *this;
}
framebufferCreateInfo& framebufferCreateInfo::AddColorFormat(format Format)
{
    this->ColorFormats.push_back(Format);
    return *this;
}
framebufferCreateInfo& framebufferCreateInfo::SetDepthFormat(format Format)
{
    this->DepthFormat = Format;
    return *this;
}
framebufferCreateInfo& framebufferCreateInfo::SetClearColor(f32 R, f32 G, f32 B, f32 A)
{
    this->ClearValues[0] = R;
    this->ClearValues[1] = G;
    this->ClearValues[2] = B;
    this->ClearValues[3] = A;
    return *this;
}
framebufferCreateInfo& framebufferCreateInfo::SetClearDepthStencil(f32 Depth, u8 Stencil)
{
    this->ClearDepth = Depth;
    this->ClearStencil = Stencil;
    return *this;
}
}