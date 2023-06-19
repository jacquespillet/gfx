#include "Gfx/Apis.h"
#define API VK

#if API == GL
#include "Gfx/OpenGL/Api.h"
#elif API == VK
#include "Gfx/Vulkan/Api.h"
#elif API == DX
#include "Gfx/D3D12/Api.h"
#endif


int main()
{
	// Create the appropriate graphics API object based on runtime configuration
	gfx::context* GfxContext = new gfx::context();

	// Initialize the graphics API
	GfxContext->Initialize();

	// Create a command buffer and swap chain
	gfx::commandBuffer *CommandBuffer = GfxContext->CreateCommandBuffer();
	gfx::swapchain *Swapchain = GfxContext->CreateSwapchain();


	// Create a vertex buffer with triangle data
	float vertices[] = {
    	-0.5f, -0.5f, 0.0f,
    	0.5f, -0.5f, 0.0f,
    	0.0f, 0.5f, 0.0f
	};

	gfx::bufferHandle vertexBuffer = GfxContext->CreateVertexBuffer(vertices, sizeof(vertices));

    gfx::pipelineHandle PipelineHandle = GfxContext->CreatePipeline("Shader.shader");
	
	gfx::renderPassHandle RenderPass = GfxContext->GetDefaultRenderPass();
	// Set other pipeline configuration parameters as needed

	// Set up the render state
	
	// Begin recording commands into the command buffer
	CommandBuffer->Begin();
	CommandBuffer->BeginPass(RenderPass);

	CommandBuffer->BindGraphicsPipeline(PipelineHandle);
	CommandBuffer->BindVertexBuffer(vertexBuffer);
	CommandBuffer->SetViewport(0, 0, 800, 600);

	// Render the triangle
	CommandBuffer->ClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	CommandBuffer->ClearBuffers(gfx::clearBufferType::Color);
	CommandBuffer->DrawTriangles(0, 3);


	CommandBuffer->EndPass();
	// End recording commands
	CommandBuffer->End();

	// Submit the command buffer to the graphics API for execution
	GfxContext->SubmitCommandBuffer(CommandBuffer);

	// Present the rendered frame
	Swapchain->Present();

	// Clean up and release resources
	GfxContext->DestroyCommandBuffer(CommandBuffer);
	GfxContext->DestroySwapchain(Swapchain);

	delete GfxContext;

	return 0;
}