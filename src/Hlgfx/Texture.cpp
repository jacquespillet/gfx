#include "Include/Texture.h"
#include "Include/Context.h"
#include "gfx/Include/Context.h"

namespace hlgfx
{
texture::texture()
{
    this->Handle = gfx::InvalidHandle;
    this->UUID = context::Get()->GetUUID();
}

texture::texture(gfx::imageHandle Handle) : Handle(Handle)
{
    this->UUID = context::Get()->GetUUID();
}

texture::~texture()
{
    gfx::context::Get()->QueueDestroyImage(this->Handle);
}
}