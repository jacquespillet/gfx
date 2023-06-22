#pragma once
#include "Types.h"

namespace gfx
{

struct shader
{
    b8 GraphicsPipeline=false;
    u32 ActiveShaders=0;
    const char *Name=nullptr;

    void *ApiData;
};

}