#pragma once
#include <vector>
#include "../Include/Types.h"

namespace gfx
{
enum class commandType 
{
    DrawTriangles,
    DrawIndexed,
    BindPipeline,
    BindVertexBuffer,
    BindIndexBuffer,
    ClearColor,
    ClearDepth,
    SetViewport,
    SetScissor,
    BeginPass,
    EndPass,
    CopyBuffer
};

struct command;
using commandFunction = void (*)(const command&);

struct command
{
    commandType Type;
    commandFunction CommandFunction;
    union
    {
        struct framebufferState
        {
            framebufferHandle FramebufferHandle;
            
            f32 ClearColor[4];
            f32 ClearDepth;
            u8 ClearStencil;
        } BeginPass;
        
        struct vertexBufferState
        {
            bufferHandle VertexBufferHandle;
        } BindVertexBuffer;
        
        struct indexBufferState
        {
            bufferHandle IndexBufferHandle;
        } BindIndexBuffer;
        
        struct drawTrianglesState
        {
            u32 Start;
            u32 Count;
            u32 InstanceCount;
        } DrawTriangles;
                
        struct drawIndexedState
        {
            u32 Count;
            u32 InstanceCount;
            indexType IndexType;
            u32 Offset;
        } DrawIndexed;
                
        struct bindBufferState
        {
            u32 Binding;
            GLuint Buffer;
        } BindUniformBuffer;
                
        struct bindImageState
        {
            u32 Binding;
            GLuint Image;
        } BindUniformImage;
        
        struct viewportState
        {
            f32 StartX;
            f32 StartY;
            f32 Width;
            f32 Height;
        } Viewport;
        
        struct scissorState
        {
            s32 StartX;
            s32 StartY;
            u32 Width;
            u32 Height;
        } Scissor;
        

        struct graphicsPipelineState
        {
            pipelineHandle Pipeline;
        } BindGraphicsPipeline;
    };
};


struct glCommandBuffer
{
    std::vector<command> Commands;
    b8 IsRecording=false;

    indexType IndexType = indexType::Uint16;
    u32 IndexBufferOffset=0;
    
};

}