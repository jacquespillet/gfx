#pragma once

#include "../Include/Uniform.h"
#include "../Include/Types.h"
#include <unordered_map>

namespace gfx
{
struct descriptorInfo
{
    
};

struct d3d12UniformData
{
    std::unordered_map<pipelineHandle, descriptorInfo> DescriptorInfos;
    b8 Initialized=false;
};    
}