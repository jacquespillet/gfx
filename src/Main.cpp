
// #if API == GL
// #include "Gfx/OpenGL/Api.h"
// #elif API == VK
// #elif API == DX
// #include "Gfx/D3D12/Api.h"
// #endif
#include <iostream>

#include "Gfx/Api.h"  
#include "App/App.h"

//TODO
//Implement buffer struct
//Implement image struct
//Implement stage buffer struct
//Implement shader struct
//Use the same shader?

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
    gfx::memory::Get()->Init();
    
	// Create the appropriate graphics API object based on runtime configuration
    uint32_t Width = 1280;
    uint32_t Height = 720;
	
	// gfx::memory::Get()->Init();
    
    app::windowCreateOptions WindowCreateOptions;
    WindowCreateOptions.Position = app::v2f(300, 100);
    WindowCreateOptions.Size = app::v2f(Width, Height);
    WindowCreateOptions.ErrorCallback = WindowErrorCallback;
    WindowCreateOptions.VersionMajor = 1;
    WindowCreateOptions.VersionMinor = 0;
    app::window Window(WindowCreateOptions);

	// Initialize the graphics API
	gfx::context::initializeInfo ContextInitialize;
	ContextInitialize.Extensions = Window.GetRequiredExtensions();
	ContextInitialize.ErrorCallback = ErrorCallback;
    ContextInitialize.InfoCallback = InfoCallback;
	std::shared_ptr<gfx::context> GfxContext = gfx::context::Initialize(ContextInitialize, Window);

	//Get the current frame command buffer
	std::shared_ptr<gfx::swapchain> Swapchain = GfxContext->CreateSwapchain(Width, Height);
	

	// Create a vertex buffer with triangle data
	// float vertices[] = {
    // 	-0.5f, -0.5f, 0.0f,
    // 	0.5f, -0.5f, 0.0f,
    // 	0.0f, 0.5f, 0.0f
	// };
    float vertices[] =
    {
        0.0f, 0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.25f, -0.25f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -0.25f, -0.25f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
    };
	gfx::bufferHandle vertexBuffer = GfxContext->CreateVertexBuffer(vertices, sizeof(vertices), 7 * sizeof(float));

    gfx::pipelineHandle PipelineHandle = GfxContext->CreatePipelineFromFile("resources/Shaders/TriangleD12.json");
    
	gfx::renderPassHandle SwapchainPass = GfxContext->GetDefaultRenderPass();

	while(!Window.ShouldClose())
	{
		Window.PollEvents();
		
		GfxContext->StartFrame();

		// Set up the render state
		std::shared_ptr<gfx::commandBuffer> CommandBuffer = GfxContext->GetCurrentFrameCommandBuffer();
		
		// Begin recording commands into the command buffer
		CommandBuffer->Begin();

		CommandBuffer->ClearColor(0.5f, 0.0f, 0.8f, 1.0f);
		CommandBuffer->ClearDepthStencil(1.0f, 0.0f);
		
		CommandBuffer->BeginPass(SwapchainPass, GfxContext->GetSwapchainFramebuffer());
		CommandBuffer->SetViewport(0, 0, Width, Height);
		CommandBuffer->SetScissor(0, 0, Width, Height);
		
		CommandBuffer->BindGraphicsPipeline(PipelineHandle);
		CommandBuffer->BindVertexBuffer(vertexBuffer);
		CommandBuffer->DrawTriangles(0, 3); 
		CommandBuffer->EndPass();

		// Submit the current frame command buffer to the graphics API for execution
		GfxContext->EndFrame();

		// Present the rendered frame
		GfxContext->Present();
	}

	// Clean up and release resources
	// GfxContext->DestroyCommandBuffer(CommandBuffer);
	// GfxContext->DestroySwapchain(Swapchain);

	GfxContext->Cleanup();

	gfx::memory *Memory = gfx::memory::Get();
	Memory->Destroy();
    delete Memory;

	return 0;
}