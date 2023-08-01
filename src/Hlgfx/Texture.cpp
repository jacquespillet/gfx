#include "Include/Texture.h"
#include "gfx/Include/Context.h"

namespace hlgfx
{
texture::texture()
{
    this->Handle = gfx::InvalidHandle;
}

texture::texture(gfx::imageHandle Handle) : Handle(Handle)
{}

texture::~texture()
{
    gfx::context::Get()->QueueDestroyImage(this->Handle);
}
}