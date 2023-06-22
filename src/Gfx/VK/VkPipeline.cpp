#if API==VK
#include "../Include/Pipeline.h"
namespace gfx
{
    
renderPassOutput &renderPassOutput::Depth(format Format, imageLayout Layout)
{
    DepthStencilFormat = Format;
    DepthStencilFinalLayout = Layout;
    return *this;
}
renderPassOutput &renderPassOutput::Color(format Format, imageLayout Layout, renderPassOperation::values Operation)
{
    ColorFinalLayouts[NumColorFormats] = Layout;
    ColorOperations[NumColorFormats] = Operation;
    ColorFormats[NumColorFormats++] = Format;
    return *this;
}
renderPassOutput &renderPassOutput::SetDepthStencilOperation(renderPassOperation::values DepthOperation, renderPassOperation::values StencilOperation)
{
    this->DepthOperation = DepthOperation;
    this->StencilOperation = StencilOperation;
    return *this;
}
void renderPassOutput::Reset()
{
    NumColorFormats = 0;
    for (u32 i = 0; i < MaxImageOutputs; i++)
    {
        ColorFormats[i] = format::UNDEFINED;
        ColorOperations[i] = renderPassOperation::DontCare;
        ColorFinalLayouts[i] = imageLayout::Undefined;
    }

    DepthStencilFormat = format::UNDEFINED;
    DepthStencilFinalLayout = imageLayout::Undefined;
} 

//

}
#endif