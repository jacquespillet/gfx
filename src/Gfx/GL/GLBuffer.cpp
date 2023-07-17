#include "../Include/Buffer.h"
#include "../Include/GfxContext.h"
#include "GLBuffer.h"
#include "GLMapping.h"
#include "GLContext.h"
#include "GLCommon.h"

#include <GL/glew.h>

namespace gfx
{
void buffer::Init(size_t ByteSize, bufferUsage::value Usage, memoryUsage MemoryUsage)
{
    std::shared_ptr<glBuffer> GLBuffer = std::static_pointer_cast<glBuffer>(ApiData);
    this->Size = ByteSize;
    
    glGenBuffers(1, &GLBuffer->Handle);
    GLBuffer->Target = BufferTargetFromUsage(Usage);
    GLBuffer->Usage = BufferUsageFromUsage(Usage);

    glBindBuffer(GLBuffer->Target, GLBuffer->Handle);
    glBufferData(GLBuffer->Target, ByteSize, nullptr, GLBuffer->Usage);
    // glBindBuffer(GLBuffer->Target, 0);
}

void buffer::CopyData(const uint8_t *Data, size_t ByteSize, size_t Offset)
{
    std::shared_ptr<glBuffer> GLBuffer = std::static_pointer_cast<glBuffer>(ApiData);
    glBindBuffer(GLBuffer->Target, GLBuffer->Handle);


#if 0
    sz CopySize = ByteSize - Offset;
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

vertexBuffer &vertexBuffer::Init()
{
    Reset();
    return *this;
}

vertexBuffer &vertexBuffer::Reset()
{
    NumVertexStreams=0;
    for (sz i = 0; i < commonConstants::MaxVertexStreams; i++)
    {
        this->VertexStreams[i].StreamIndex = (u32)-1;
    }
    ApiData = nullptr;

    return *this;
}

vertexBuffer &vertexBuffer::AddVertexStream(vertexStreamData StreamData)
{
    this->VertexStreams[NumVertexStreams++] = StreamData;   
    return *this;
}

vertexBuffer &vertexBuffer::Create()
{
    GET_CONTEXT(GLData, context::Get());

    this->ApiData = std::make_shared<glVertexBuffer>();
    std::shared_ptr<glVertexBuffer> GLVertexBuffer = std::static_pointer_cast<glVertexBuffer>(this->ApiData);

    //Create the vao
    glGenVertexArrays(1, &GLVertexBuffer->VAO);

    glBindVertexArray(GLVertexBuffer->VAO);
    //Create the vbos
    for(sz i=0; i<NumVertexStreams; i++)
    {
        GLVertexBuffer->VertexBuffers[i] = context::Get()->ResourceManager.Buffers.ObtainResource();
        buffer *Buffer = (buffer*)context::Get()->ResourceManager.Buffers.GetResource(GLVertexBuffer->VertexBuffers[i]);
        Buffer->ApiData = std::make_shared<glBuffer>();

        Buffer->Init(VertexStreams[i].Size, bufferUsage::VertexBuffer, memoryUsage::GpuOnly);
        GLData->CheckErrors();
        Buffer->CopyData((u8*)VertexStreams[i].Data, VertexStreams[i].Size, 0);
        GLData->CheckErrors();
        std::shared_ptr<glBuffer> GLBuffer = std::static_pointer_cast<glBuffer>(Buffer->ApiData);
        
        glBindBuffer(GLBuffer->Target, GLBuffer->Handle);
        
        u32 StartPtr=0;
        for(u32 j=0; j<VertexStreams[i].AttributesCount; j++)
        {
            glVertexAttribPointer(VertexStreams[i].InputAttributes[j].InputIndex, 
                                VertexStreams[i].InputAttributes[j].ElementCount, 
                                VertexAttributeTypeToNative(VertexStreams[i].InputAttributes[j].Type), 
                                VertexStreams[i].InputAttributes[j].Normalized,
                                (GLsizei)VertexStreams[i].Stride, 
                                (void*)((uintptr_t)StartPtr));
            glEnableVertexAttribArray(VertexStreams[i].InputAttributes[j].InputIndex);
            if(VertexStreams[i].InputRate == vertexInputRate::PerInstance) glVertexAttribDivisor(VertexStreams[i].InputAttributes[j].InputIndex, 1); 
            StartPtr += VertexStreams[i].InputAttributes[j].ElementCount * VertexStreams[i].InputAttributes[j].ElementSize;
        }
        glBindBuffer(GLBuffer->Target, 0);
    }
    glBindVertexArray(0);
    GLData->CheckErrors();

    

    


    // GET_CONTEXT(GLData, context::Get());
    // for(sz i=0; i<NumVertexStreams; i++)
    // {
    //     std::vector<vertexInputAttribute> Attributes(VertexStreams[i].AttributesCount);
    //     memcpy(&Attributes[0], &VertexStreams[i].InputAttributes, VertexStreams[i].AttributesCount * sizeof(vertexInputAttribute));

    //     VertexStreams[i].Buffer = context::Get()->CreateVertexBuffer((f32*)VertexStreams[i].Data, VertexStreams[i].Size, VertexStreams[i].Stride, Attributes);
    // }
    
    return *this;
}

}