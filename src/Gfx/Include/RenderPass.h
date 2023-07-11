#pragma once
#include <memory>
#include <string>
#include "Types.h"

namespace gfx
{
static const u8 MaxImageOutputs = 8;

namespace renderPassOperation
{
enum values
{
    DontCare, Load, Clear, Count
};
}

struct renderPassOutput
{
    format ColorFormats[MaxImageOutputs];
    imageLayout ColorFinalLayouts[MaxImageOutputs];
    renderPassOperation::values ColorOperations[MaxImageOutputs];

    format DepthStencilFormat;
    imageLayout DepthStencilFinalLayout;

    u32 NumColorFormats=0;

    renderPassOperation::values DepthOperation = renderPassOperation::DontCare;
    renderPassOperation::values StencilOperation = renderPassOperation::DontCare;

    const char *Name;

    void Reset();
    renderPassOutput &Depth(format Format, imageLayout Layout);
    renderPassOutput &Color(format Format, imageLayout Layout, renderPassOperation::values Operation);
    renderPassOutput &SetDepthStencilOperation(renderPassOperation::values DepthOperation, renderPassOperation::values StencilOperation);
};

struct renderPass
{  
    std::shared_ptr<void> ApiData;
    std::string Name;
    renderPassOutput Output;
};
}