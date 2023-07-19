#include "../Include/GfxContext.h"
#include "../Include/Shader.h"

namespace gfx
{
buffer *context::GetBuffer(bufferHandle Handle)
{
    return (buffer*) this->ResourceManager.Buffers.GetResource(Handle);
}

vertexBuffer *context::GetVertexBuffer(vertexBufferHandle Handle)
{
    return (vertexBuffer*) this->ResourceManager.VertexBuffers.GetResource(Handle);
}

image *context::GetImage(imageHandle Handle)
{
    return (image*) this->ResourceManager.Images.GetResource(Handle);
}

pipeline *context::GetPipeline(pipelineHandle Handle)
{
    return (pipeline*) this->ResourceManager.Pipelines.GetResource(Handle);
}

shader *context::GetShader(shaderStateHandle Handle)
{
    return (shader*) this->ResourceManager.Shaders.GetResource(Handle);
}

renderPass *context::GetRenderPass(renderPassHandle Handle)
{
    return (renderPass*) this->ResourceManager.RenderPasses.GetResource(Handle);
}

framebuffer *context::GetFramebuffer(framebufferHandle Handle)
{
    return (framebuffer*) this->ResourceManager.Framebuffers.GetResource(Handle);
}   

}