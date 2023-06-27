#if API == D3D12

#include "../../App/App.h"
#include "../Include/GfxContext.h"
#include "../Include/Image.h"
#include "../Include/CommandBuffer.h"
#include "../Include/Swapchain.h"
#include "../Include/Buffer.h"
#include "../Include/Pipeline.h"
#include "../Include/Shader.h"
#include "../Include/RenderPass.h"
#include "../Include/Framebuffer.h"
#include "../Include/Memory.h"
#include "../Common/Util.h"

namespace gfx
{

context* context::Singleton= nullptr;

context *context::Get()
{
    return Singleton;
}

pipelineHandle context::CreatePipeline(const pipelineCreation &PipelineCreation)
{
    return 0;
}



}

#endif