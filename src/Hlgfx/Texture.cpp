#include "Include/Texture.h"
#include "Include/Context.h"
#include "gfx/Include/Context.h"

namespace hlgfx
{
texture::texture(std::string Name)
{
    this->Name = Name;
    this->Handle = gfx::InvalidHandle;
    this->UUID = context::Get()->GetUUID();
}

texture::texture(std::string Name, gfx::imageHandle Handle) : Handle(Handle)
{
    this->Name = Name;
    this->UUID = context::Get()->GetUUID();
}

texture::~texture()
{
    gfx::context::Get()->QueueDestroyImage(this->Handle);
}
}