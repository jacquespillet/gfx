#include "../Include/ResourceManager.h"
#include "../Include/Buffer.h"
#include "../Include/Image.h"
#include "../Include/Pipeline.h"
#include "../Include/Shader.h"
#include "../Include/RenderPass.h"
#include "../Include/Framebuffer.h"
#include "../Include/AccelerationStructure.h"

namespace gfx
{
void resourceManager::Init()
{
    Buffers.Init(2048, sizeof(buffer));
	Images.Init(2048, sizeof(image));
	Pipelines.Init(2048, sizeof(pipeline));
	Shaders.Init(128, sizeof(shader));
	RenderPasses.Init(128, sizeof(renderPass));
	Framebuffers.Init(128, sizeof(framebuffer));
	VertexBuffers.Init(2048, sizeof(vertexBuffer));
#if GFX_API == GFX_VK || GFX_API == GFX_D3D12
	AccelerationStructures.Init(512, sizeof(accelerationStructure));
#endif
	InitApiSpecific();
}

void resourceManager::Destroy()
{
    printf("Destroying Buffers\n");
	Buffers.Destroy();
	printf("Destroying Images\n");
	Images.Destroy();
	printf("Destroying Pipelines\n");
	Pipelines.Destroy();
	printf("Destroying Shaders\n");
	Shaders.Destroy();
	printf("Destroying RenderPasses\n");
	RenderPasses.Destroy();
	printf("Destroying Framebuffers\n");
	Framebuffers.Destroy();
	printf("Destroying VertexBuffers\n");
	VertexBuffers.Destroy();
#if GFX_API == GFX_VK || GFX_API == GFX_D3D12
	printf("Destroying Acceleration Structures\n");
	AccelerationStructures.Destroy();
#endif

	DestroyApiSpecific();
}


}