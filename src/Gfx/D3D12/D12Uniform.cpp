#include "../Include/Uniform.h"
#include "../Include/GfxContext.h"
#include "D12Uniform.h"
#include "D12Common.h"
#include "D12GfxContext.h"

namespace gfx
{
uniformGroup &uniformGroup::Reset()
{
    return *this;
}

uniformGroup &uniformGroup::Update()
{
    return *this;
}

}