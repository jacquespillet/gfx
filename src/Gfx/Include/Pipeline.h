#pragma once

#include "Types.h"

#include <filesystem>
#include <json.hpp>
#include <regex>
#include "../Common/Util.h"
#include <memory>
#include "RenderPass.h"

// 	// Create a shader program
// 	ShaderHandle vertexShader = GfxContext->CreateShader("vertex_shader.glsl", ShaderType::Vertex);
// 	ShaderHandle fragmentShader = GfxContext->CreateShader("fragment_shader.glsl", ShaderType::Fragment);
// 	ProgramHandle shaderProgram = GfxContext->CreateProgram(vertexShader, fragmentShader);

// 	// Create a graphics pipeline
// 	GraphicsPipelineHandle pipeline = GfxContext->CreateGraphicsPipeline();
// 	GfxContext->SetVertexShader(pipeline, vertexShader);
// 	GfxContext->SetFragmentShader(pipeline, fragmentShader);

namespace gfx
{




struct vertexStream
{
    u16 Binding=0;
    u16 Stride=0;
    vertexInputRate InputRate = vertexInputRate::Count;
};

namespace vertexComponentFormat
{
enum values
{
    Float, Float2, Float3, Float4, Mat4, Byte, Byte4N, UByte, UByte4N, Short2, Short2N, Short4, Short4N, Uint, Uint2, Uint4, Count
};
static const char *StringValues[] =
{
    "Float", "Float2", "Float3", "Float4", "Mat4", "Byte", "Byte4N", "UByte", "UByte4N", "Short2", "Short2N", "Short4", "Short4N", "Uint", "Uint2", "Uint4", "Count"
};
static const char *ToString(values Values)
{
    return StringValues[(sz)Values];
}
}


struct vertexAttribute
{
    u16 Location=0;
    u16 Binding=0;
    u32 Offset=0;
    u32 SemanticIndex=0;
    vertexComponentFormat::values Format = vertexComponentFormat::Count;
};

struct vertexInputCreation
{
    u32 NumVertexStreams=0;
    u32 NumVertexAttributes=0;

    vertexStream VertexStreams[commonConstants::MaxVertexStreams];
    vertexAttribute VertexAttributes[commonConstants::MaxVertexAttributes];

    vertexInputCreation &Reset();
    vertexInputCreation &AddVertexStream(const vertexStream &Stream);
    vertexInputCreation &AddVertexAttribute(const vertexAttribute &Attribute);
};


struct stencilOperationState
{
    stencilOperation Fail = stencilOperation::Keep;
    stencilOperation Pass = stencilOperation::Keep;
    stencilOperation DepthFail = stencilOperation::Keep;
    compareOperation Compare = compareOperation::Always;

    u32 CompareMask = 0xff;
    u32 WriteMask = 0xff;
    u32 Reference = 0xff;
};

struct depthStencilCreation
{
    stencilOperationState Front;
    stencilOperationState Back;

    compareOperation DepthComparison = compareOperation::Always;

    u8 DepthEnable : 1;
    u8 DepthWriteEnable : 1;
    u8 StencilEnable : 1;
    u8 Pad : 1;

    depthStencilCreation &SetDepth(b8 Write, compareOperation CompareOp);
    depthStencilCreation & Reset();
};




struct shaderStage
{
    const char *FileName = nullptr;
    const char *Code = nullptr;
    u32 CodeSize = 0;
    shaderStageFlags::bits Stage = shaderStageFlags::All;
};

struct shaderStateCreation
{
    shaderStage Stages[commonConstants::MaxShaderStages];
    const char *Name=nullptr;
    u32 StagesCount=0;
    u32 SpvInput=0;
    shaderStateCreation &Reset();
    shaderStateCreation &SetName(const char *);
    shaderStateCreation &AddStage(const char *Code, const char *FileName, u32 CodeSize, shaderStageFlags::bits Stage);
    shaderStateCreation &AddStage(const char *FileName, shaderStageFlags::bits Stage);
};


namespace colorWriteEnabled
{
    enum values
    {
        Red, Green, Blue, Alpha, All, Count
    };

    enum mask 
    {
        RedMask = 1 << 0,
        GreenMask = 1 << 1,
        BlueMask = 1 << 2,
        AlphaMask = 1 << 3,
        AllMask = RedMask | GreenMask | BlueMask | AlphaMask
    };
}

struct blendState
{
    blendFactor SourceColor = blendFactor::One;
    blendFactor DestinationColor = blendFactor::One;
    blendOperation ColorOp = blendOperation::Add;
    
    blendFactor SourceAlpha = blendFactor::One;
    blendFactor DestinationAlpha = blendFactor::One;
    blendOperation AlphaOp = blendOperation::Add;

    colorWriteEnabled::mask ColorWriteMask = colorWriteEnabled::AllMask;

    u8 BlendEnabled : 1;
    u8 SeparateBlend : 1;
    u8 Pad : 6;

    blendState &SetColor(blendFactor Source, blendFactor Dest, blendOperation Operation);

};

struct blendStateCreation
{
    blendState BlendStates[commonConstants::MaxImageOutputs];
    u32 ActiveStates =0;
    blendState &AddBlendState();
};




struct rasterizationCreation
{
    cullMode::bits CullMode = cullMode::None;
    frontFace FrontFace = frontFace::CounterClockwise;
    fillMode Fill = fillMode::Solid;
};

struct pipelineCreation
{
    vertexInputCreation VertexInput;
    depthStencilCreation DepthStencil;
    renderPassOutput RenderPass;
    shaderStateCreation Shaders;
    blendStateCreation BlendState;
    rasterizationCreation Rasterization;

    b8 IsCompute = false;
    const char *Name;
};

struct graphicsPipeline
{
    renderPassHandle RenderPass;
};


void ShaderConcatenate(std::string &FileName, std::string &Code, std::string &ParentPath);
const char *AllocateCString(std::string Str);

blendFactor GetBlendFactor( const std::string factor );
blendOperation GetBlendOp( const std::string op );

void ParseGPUPipeline(nlohmann::json &PipelineJSON, pipelineCreation &PipelineCreation, std::string &ParentPath);

struct pipeline
{
    depthStencilCreation DepthStencil;
    blendStateCreation  BlendState;
    rasterizationCreation Rasterization;
    
    b8 GraphicsPipeline=true;

    framebufferHandle Framebuffer;

    std::shared_ptr<void> ApiData;
};


}