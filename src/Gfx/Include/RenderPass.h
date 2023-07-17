#pragma once
#include <memory>
#include <string>
#include "Types.h"

namespace gfx
{

namespace renderPassOperation
{
enum values
{
    DontCare, Load, Clear, Count
};
}

struct renderPassOutput
{
    format ColorFormats[commonConstants::MaxImageOutputs];
    imageLayout ColorFinalLayouts[commonConstants::MaxImageOutputs];
    renderPassOperation::values ColorOperations[commonConstants::MaxImageOutputs];

    format DepthStencilFormat;
    imageLayout DepthStencilFinalLayout;

    u32 NumColorFormats=0;

    renderPassOperation::values DepthOperation = renderPassOperation::DontCare;
    renderPassOperation::values StencilOperation = renderPassOperation::DontCare;

    u32 SampleCount = 1;
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