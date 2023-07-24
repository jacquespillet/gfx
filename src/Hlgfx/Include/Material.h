#pragma once
#include "Gfx/Api.h"

namespace hlgfx
{
struct material
{
gfx::pipelineHandle PipelineHandle;
};

struct unlitMaterial : public material
{
    unlitMaterial();
};

}