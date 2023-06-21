#include "../Include/ResourceManager.h"
#include "../Include/Buffer.h"
#include "../Include/Image.h"

namespace gfx
{
void resourceManager::Init()
{
    Buffers.Init(2048, sizeof(buffer));
	Images.Init(2048, sizeof(image));
}
}