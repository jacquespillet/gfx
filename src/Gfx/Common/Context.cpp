#include "../Include/Context.h"
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
    if(!this->ResourceManager.Images.Initialized) return nullptr;
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

void context::QueueDestroyBuffer(bufferHandle Buffer)
{
    this->ResourceDeletionQueue.push_back({Buffer, resourceDeletion::type::Buffer});
}
void context::QueueDestroyVertexBuffer(vertexBufferHandle Buffer)
{
    this->ResourceDeletionQueue.push_back({Buffer, resourceDeletion::type::VertexBuffer});
}
void context::QueueDestroyPipeline(pipelineHandle Pipeline)
{
    this->ResourceDeletionQueue.push_back({Pipeline, resourceDeletion::type::Pipeline});
}
void context::QueueDestroyImage(imageHandle Image)
{
    this->ResourceDeletionQueue.push_back({Image, resourceDeletion::type::Image});
}

void context::ProcessDeletionQueue()
{
    for (sz i = 0; i < this->ResourceDeletionQueue.size(); i++)
    {
        if(this->ResourceDeletionQueue[i].Type == resourceDeletion::type::Buffer)
        {
            this->DestroyBuffer(this->ResourceDeletionQueue[i].Handle);
        }
        else if(this->ResourceDeletionQueue[i].Type == resourceDeletion::type::VertexBuffer)
        {
            this->DestroyVertexBuffer(this->ResourceDeletionQueue[i].Handle);
        } 
        else if(this->ResourceDeletionQueue[i].Type == resourceDeletion::type::Pipeline)
        {
            this->DestroyPipeline(this->ResourceDeletionQueue[i].Handle);
        } 
        else if(this->ResourceDeletionQueue[i].Type == resourceDeletion::type::Image)
        {
            this->DestroyImage(this->ResourceDeletionQueue[i].Handle);
        } 
    }
    this->ResourceDeletionQueue.clear();    
}

}