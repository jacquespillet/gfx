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
        struct
        {
            framebufferHandle FramebufferHandle;
        } BeginPass;
        
        struct
        {
            bufferHandle VertexBufferHandle;
        } BindVertexBuffer;
        
        struct
        {
            u32 Start;
            u32 Count;
        } DrawTriangles;
        
        struct
        {
            pipelineHandle Pipeline;
        } BindGraphicsPipeline;
    };
};


struct glCommandBuffer
{
    std::vector<command> Commands;
    bool IsRecording=false;
};

}