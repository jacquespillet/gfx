
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
    gfx::memory::Get()->Init();
    
	// Create the appropriate graphics API object based on runtime configuration
    uint32_t Width = 1280;
    uint32_t Height = 720;
	
	// gfx::memory::Get()->Init();
    
    app::windowCreateOptions WindowCreateOptions;
    WindowCreateOptions.Position = app::v2f(300, 100);
    WindowCreateOptions.Size = app::v2f(Width, Height);
    WindowCreateOptions.ErrorCallback = WindowErrorCallback;
#if GFX_API == GFX_VK
	WindowCreateOptions.VersionMajor = 1;
    WindowCreateOptions.VersionMinor = 0;
#elif GFX_API == GFX_GL
	WindowCreateOptions.VersionMajor = 4;
    WindowCreateOptions.VersionMinor = 5;
#endif
    app::window Window(WindowCreateOptions);

	// Initialize the graphics API
	gfx::context::initializeInfo ContextInitialize;
	ContextInitialize.Extensions = Window.GetRequiredExtensions();
	ContextInitialize.ErrorCallback = ErrorCallback;
    ContextInitialize.InfoCallback = InfoCallback;
	std::shared_ptr<gfx::context> GfxContext = gfx::context::Initialize(ContextInitialize, Window);

	//TODO : Do we really need that ? May as well create it in the context initialization...
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
	
	std::vector<gfx::vertexInputAttribute> Attributes = 
	{
		{sizeof(float), 3, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION},
		{sizeof(float), 4, gfx::vertexAttributeType::Float, true, gfx::attributeSemantic::COLOR}
	};
	gfx::bufferHandle VertexBuffer = GfxContext->CreateVertexBuffer(vertices, sizeof(vertices), 7 * sizeof(float), Attributes);

    gfx::pipelineHandle PipelineHandle = GfxContext->CreatePipelineFromFile("resources/Shaders/Triangle.json");
    
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
		CommandBuffer->BindVertexBuffer(VertexBuffer);
		CommandBuffer->DrawTriangles(0, 3); 
		CommandBuffer->EndPass();
		
		GfxContext->EndFrame();

		// Present the rendered frame
		GfxContext->Present();
	}


	// GfxContext->WaitIdle();
	// GfxContext->DestroyPipeline(PipelineHandle);
	// GfxContext->DestroyBuffer(VertexBuffer);

	// GfxContext->DestroySwapchain();
	// GfxContext->Cleanup();

	gfx::memory *Memory = gfx::memory::Get();
	Memory->Destroy();
    delete Memory;


	system("pause");
	//TODO: Investigate crash here ?
	return 0;
}