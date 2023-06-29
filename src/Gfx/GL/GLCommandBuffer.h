#pragma once
#include <vector>
#include "../Include/Types.h"

namespace gfx
{
enum class commandType 
{
    DrawTriangles,
    BindPipeline,
    BindVertexBuffer,
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
        } BeginPass;
        
        struct vertexBufferState
        {
            bufferHandle VertexBufferHandle;
        } BindVertexBuffer;
        
        struct drawTrianglesState
        {
            u32 Start;
            u32 Count;
        } DrawTriangles;
        
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
        
        struct clearState
        {
            f32 ClearColor[4];
            f32 ClearDepth;
            u8 ClearStencil;
        } Clear;

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

    
};

}