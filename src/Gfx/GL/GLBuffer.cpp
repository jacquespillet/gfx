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
#if 0
    if(CopySize == this->Size)
    {
        glBufferData(GLBuffer->Target, CopySize, Data, GLBuffer->Usage);
    }
    else
    {
        glBufferSubData(GLBuffer->Target, Offset, ByteSize, Data);
    }
#else
    if(this->MappedData == nullptr)
    {
        this->MappedData = MapMemory();
        memcpy(this->MappedData + Offset, Data, ByteSize);
        UnmapMemory();
    }
    else
    {
        memcpy(this->MappedData + Offset, Data, ByteSize);
    }

#endif
    glBindBuffer(GLBuffer->Target, 0);
}

u8 *buffer::MapMemory()
{
    std::shared_ptr<glBuffer> GLBuffer = std::static_pointer_cast<glBuffer>(this->ApiData);

    //If unmapped, map it
    if(this->MappedData == nullptr)
    {
        this->MappedData = (u8*)glMapBuffer(GLBuffer->Target, GL_WRITE_ONLY);
    }
    if(this->MappedData == nullptr) assert(false);
    return this->MappedData;    
}

void buffer::UnmapMemory()
{
    std::shared_ptr<glBuffer> GLBuffer = std::static_pointer_cast<glBuffer>(this->ApiData);
    glUnmapBuffer(GLBuffer->Target);
    this->MappedData=nullptr;
}

}