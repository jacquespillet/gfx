#include "../Include/ResourceManager.h"
#include "../Include/Buffer.h"
#include "../Include/Image.h"
#include "../Include/Pipeline.h"
#include "../Include/Shader.h"
#include "../Include/RenderPass.h"
#include "../Include/Framebuffer.h"

namespace gfx
{
void resourceManager::Init()
{
    Buffers.Init(2048, sizeof(buffer));
	Images.Init(2048, sizeof(image));
	Pipelines.Init(2048, sizeof(pipeline));
	Shaders.Init(2048, sizeof(shader));
	RenderPasses.Init(2048, sizeof(renderPass));
	Framebuffers.Init(2048, sizeof(framebuffer));

	InitApiSpecific();
}

void resourceManager::Destroy()
{
    Buffers.Destroy();
	Images.Destroy();
	Pipelines.Destroy();
	Shaders.Destroy();
	RenderPasses.Destroy();
	Framebuffers.Destroy();

	DestroyApiSpecific();
}


}