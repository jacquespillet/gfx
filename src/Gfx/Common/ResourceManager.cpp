#include "../Include/ResourceManager.h"
#include "../Include/Buffer.h"
#include "../Include/Image.h"
#include "../Include/Pipeline.h"
#include "../Include/Shader.h"

namespace gfx
{
void resourceManager::Init()
{
    Buffers.Init(2048, sizeof(buffer));
	Images.Init(2048, sizeof(image));
	Pipelines.Init(2048, sizeof(pipeline));
	Shaders.Init(2048, sizeof(shader));

	InitApiSpecific();
}
}