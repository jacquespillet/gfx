#include "../Include/Uniform.h"
#include "../Include/GfxContext.h"
#include "D12Uniform.h"
#include "D12Common.h"
#include "D12GfxContext.h"

namespace gfx
{
void uniformGroup::Initialize()
{
    this->ApiData = std::make_shared<d3d12UniformData>();
}

void uniformGroup::Update()
{

}

}