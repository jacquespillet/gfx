
// #if API == GL
// #include "Gfx/OpenGL/Api.h"
// #elif API == VK
// #elif API == DX
// #include "Gfx/D3D12/Api.h"
// #endif
#include <iostream>

#include "Gfx/Api.h"
#include "App/App.h"

void WindowErrorCallback(const std::string &errorMessage)
{
    std::cout << "Window Error : " << errorMessage << std::endl;
}

void ErrorCallback(const std::string &message)
{
    std::cout << " Error : " << message << std::endl;
}

void InfoCallback(const std::string &message)
{
    std::cout << " Info : " << message << std::endl;
}

int main()
{	
    uint32_t Width = 1280;
    uint32_t Height = 720;

    app::windowCreateOptions WindowCreateOptions;
    WindowCreateOptions.Position = app::v2f(300, 100);
    WindowCreateOptions.Size = app::v2f(Width, Height);
    WindowCreateOptions.ErrorCallback = WindowErrorCallback;
    WindowCreateOptions.VersionMajor = 1;
    WindowCreateOptions.VersionMinor = 0;
    app::window Window(WindowCreateOptions);

	// Create the appropriate graphics API object based on runtime configuration
	
	// Initialize the graphics API
	gfx::context::initializeInfo ContextInitialize;
	ContextInitialize.Extensions = Window.GetRequiredExtensions();
	ContextInitialize.ErrorCallback = ErrorCallback;
    ContextInitialize.InfoCallback = InfoCallback;
	gfx::context *GfxContext = gfx::context::Initialize(ContextInitialize, Window);

	// Create a command buffer and swap chain
	gfx::commandBuffer *CommandBuffer = GfxContext->CreateCommandBuffer();
	gfx::swapchain *Swapchain = GfxContext->CreateSwapchain(Width, Height);
	

	// Create a vertex buffer with triangle data
	float vertices[] = {
    	-0.5f, -0.5f, 0.0f,
    	0.5f, -0.5f, 0.0f,
    	0.0f, 0.5f, 0.0f
	};

	gfx::bufferHandle vertexBuffer = GfxContext->CreateVertexBuffer(vertices, sizeof(vertices));

    gfx::pipelineHandle PipelineHandle = GfxContext->CreatePipelineFromFile("resources/Shaders/Triangle.json");
	
#if 0
	gfx::renderPassHandle RenderPass = GfxContext->GetDefaultRenderPass();
	// Set other pipeline configuration parameters as needed

	while(!Window.ShouldClose())
	{
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
	}

	// Clean up and release resources
	GfxContext->DestroyCommandBuffer(CommandBuffer);
	GfxContext->DestroySwapchain(Swapchain);

	delete GfxContext;
#endif
	return 0;
}