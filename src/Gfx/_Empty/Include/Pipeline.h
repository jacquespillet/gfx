#pragma once


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
struct graphicsPipeline
{
    renderPassHandle RenderPass;
};
}