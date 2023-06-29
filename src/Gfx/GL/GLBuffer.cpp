#include "../Include/Buffer.h"
#include "GLBuffer.h"
#include "GLMapping.h"

#include <GL/glew.h>

namespace gfx
{
void buffer::Init(size_t ByteSize, bufferUsage::value Usage, memoryUsage MemoryUsage)
{
    std::shared_ptr<glBuffer> GLBuffer = std::static_pointer_cast<glBuffer>(ApiData);
    this->Size = ByteSize;
    
    glGenBuffers(1, &GLBuffer->Handle);
    if(Usage == bufferUsage::VertexBuffer)
    {
        glGenVertexArrays(1, &GLBuffer->VAO);
    }

    GLBuffer->Target = BufferTargetFromUsage(Usage);
    GLBuffer->Usage = BufferUsageFromUsage(Usage);

    glBindBuffer(GLBuffer->Target, GLBuffer->Handle);
    glBufferData(GLBuffer->Target, ByteSize, nullptr, GLBuffer->Usage);
    glBindBuffer(GLBuffer->Target, 0);
}

void buffer::CopyData(const uint8_t *Data, size_t ByteSize, size_t Offset)
{
    std::shared_ptr<glBuffer> GLBuffer = std::static_pointer_cast<glBuffer>(ApiData);
    glBindBuffer(GLBuffer->Target, GLBuffer->Handle);
    
    u32 CopySize = ByteSize - Offset;
    if(CopySize == this->Size)
    {
        glBufferData(GLBuffer->Target, CopySize, Data, GLBuffer->Usage);
    }
    else
    {
        glBufferSubData(GLBuffer->Target, Offset, ByteSize, Data);
    }
    glBindBuffer(GLBuffer->Target, 0);
}

}