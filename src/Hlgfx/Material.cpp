#include "Include/Material.h"

namespace hlgfx
{

unlitMaterial::unlitMaterial()
{
    PipelineHandle = gfx::context::Get()->CreatePipelineFromFile("resources/Hlgfx/Shaders/Unlit/Unlit.json");
}

}