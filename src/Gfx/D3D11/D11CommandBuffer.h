#pragma once
#include <vector>
#include "../Include/Types.h"

#include <d3d11_1.h>

namespace gfx
{
enum class commandType 
{
    DrawTriangles,
    DrawIndexed,
    BindPipeline,
    BindVertexBuffer,
    BindIndexBuffer,
    BindUniforms,
    ClearColor,
    ClearDepth,
    SetViewport,
    SetScissor,
    BeginPass,
    EndPass,
    CopyBuffer,
    Dispatch,
    DrawImgui
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
            indexType IndexType;
            u32 Offset;
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
            ID3D11Buffer* Buffer;
        } BindUniformBuffer;
                
        struct bindStorageBufferState
        {
            u32 Binding;
            ID3D11ShaderResourceView* Buffer;
        } BindStorageBuffer;
                
        struct bindImageState
        {
            u32 Binding;
            ID3D11ShaderResourceView* Image;
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
        
        struct dispatchComputeState
        {
            u32 NumGroupX, NumGroupY, NumGroupZ;
        } DispatchCompute;

    };
};


struct d3d11CommandBuffer
{
    std::vector<command> Commands;
    b8 IsRecording=false;

    indexType IndexType = indexType::Uint16;
    u32 IndexBufferOffset=0;

    void DrawImgui();
    
};

}