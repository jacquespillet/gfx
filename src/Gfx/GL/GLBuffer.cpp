#include "../Include/Buffer.h"
#include "../Include/Context.h"
#include "GLBuffer.h"
#include "GLMapping.h"
#include "GLContext.h"
#include "GLCommon.h"

#include <glad/gl.h>

namespace gfx
{
void buffer::Init(size_t ByteSize, sz Stride, bufferUsage::value Usage, memoryUsage MemoryUsage)
{
    GET_API_DATA(GLBuffer, glBuffer, this); 
    
    this->Size = ByteSize;
    this->Stride = Stride;
    this->MemoryUsage = MemoryUsage;

    glGenBuffers(1, &GLBuffer->Handle);
    GLBuffer->Target = BufferTargetFromUsage(Usage);
    GLBuffer->Usage = BufferUsageFromUsage(Usage);

    glBindBuffer(GLBuffer->Target, GLBuffer->Handle);
    glBufferData(GLBuffer->Target, ByteSize, nullptr, GLBuffer->Usage);
    // glBindBuffer(GLBuffer->Target, 0);
}

void buffer::CopyData(const uint8_t *Data, size_t ByteSize, size_t Offset)
{
    GET_API_DATA(GLBuffer, glBuffer, this); 
    glBindBuffer(GLBuffer->Target, GLBuffer->Handle);

    if(MemoryUsage == memoryUsage::GpuOnly)
    {
        sz CopySize = ByteSize - Offset;
        if(CopySize == this->Size)
        {
            glBufferData(GLBuffer->Target, CopySize, Data, GLBuffer->Usage);
        }
        else
        {
            glBufferSubData(GLBuffer->Target, Offset, ByteSize, Data);
        }
    }
    else
    {
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
    }
    glBindBuffer(GLBuffer->Target, 0);
}

u8 *buffer::MapMemory()
{
    GET_API_DATA(GLBuffer, glBuffer, this); 

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
    GET_API_DATA(GLBuffer, glBuffer, this); 
    glUnmapBuffer(GLBuffer->Target);
    this->MappedData=nullptr;
}


vertexStreamData &vertexStreamData::Reset()
{
    Data=nullptr;
    Size=0;
    Stride=0;
    StreamIndex=0;
    Buffer=0;
    AttributesCount=0;
    return *this;
}

vertexStreamData &vertexStreamData::SetData(void *Data)
{
    this->Data = Data;
    return *this;
}

vertexStreamData &vertexStreamData::SetInputRate(vertexInputRate InputRate)
{
    this->InputRate = InputRate;
    return *this;
}

vertexStreamData &vertexStreamData::SetSize(u32 Size)
{
    this->Size = Size;
    return *this;
}
vertexStreamData &vertexStreamData::SetStride(u32 Stride)
{
    this->Stride = Stride;
    return *this;
}
vertexStreamData &vertexStreamData::SetStreamIndex(u32 StreamIndex)
{
    this->StreamIndex = StreamIndex;
    return *this;
}
vertexStreamData &vertexStreamData::AddAttribute(vertexInputAttribute Attribute)
{
    this->InputAttributes[this->AttributesCount++] = Attribute;
    return *this;
}

vertexBufferCreateInfo &vertexBufferCreateInfo::Init()
{
    Reset();
    return *this;
}

vertexBufferCreateInfo &vertexBufferCreateInfo::Reset()
{
    NumVertexStreams=0;
    for (sz i = 0; i < commonConstants::MaxVertexStreams; i++)
    {
        this->VertexStreams[i].StreamIndex = (u32)-1;
    }

    return *this;
}

vertexBufferCreateInfo &vertexBufferCreateInfo::AddVertexStream(vertexStreamData StreamData)
{
    this->VertexStreams[NumVertexStreams++] = StreamData;   
    return *this;
}


}