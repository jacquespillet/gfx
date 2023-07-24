#include "Include/Material.h"
#include "Include/Context.h"

namespace hlgfx
{

unlitMaterial::unlitMaterial()
{
    PipelineHandle = context::Get()->Pipelines[context::UnlitPipeline];
}

}