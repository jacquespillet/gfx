#pragma once
#include "Types.h"
#include <memory>
namespace gfx
{

struct shader
{
    b8 ComputePipeline=false;
    b8 RTXPipeline=false;
    u32 ActiveShaders=0;
    const char *Name=nullptr;

    std::shared_ptr<void> ApiData;
};

}