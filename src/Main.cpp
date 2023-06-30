
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

void OnResizeWindow(app::window &Window, app::v2i NewSize);

struct application
{
	std::shared_ptr<app::window> Window;
	std::shared_ptr<gfx::context> GfxContext;
	gfx::renderPassHandle SwapchainPass;
	gfx::pipelineHandle PipelineHandle;
	gfx::bufferHandle VertexBuffer;
	std::shared_ptr<gfx::swapchain> Swapchain;
	std::shared_ptr<gfx::uniformGroup> Uniforms;


	struct uniformData
	{
		gfx::v4f Color0;
		gfx::v4f Color1;
	};
	uniformData UniformData;

	uint32_t Width, Height;
	void Init()
	{
		UniformData = 
		{
			gfx::v4f(1,0,0,1),
			gfx::v4f(0,1,0,1)
		};
		
		gfx::memory::Get()->Init();
    
		// Create the appropriate graphics API object based on runtime configuration
		Width = 1280;
		Height = 720;
		
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
		Window = std::make_shared<app::window>(WindowCreateOptions);
		Window->OnResize = OnResizeWindow;

		// Initialize the graphics API
		gfx::context::initializeInfo ContextInitialize;
		ContextInitialize.Extensions = Window->GetRequiredExtensions();
		ContextInitialize.ErrorCallback = ErrorCallback;
		ContextInitialize.InfoCallback = InfoCallback;
		GfxContext = gfx::context::Initialize(ContextInitialize, *Window);

		//TODO : Do we really need that ? May as well create it in the context initialization...
		Swapchain = GfxContext->CreateSwapchain(Width, Height);
		

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
		VertexBuffer = GfxContext->CreateVertexBuffer(vertices, sizeof(vertices), 7 * sizeof(float), Attributes);

		PipelineHandle = GfxContext->CreatePipelineFromFile("resources/Shaders/Triangle.json");
		
		SwapchainPass = GfxContext->GetDefaultRenderPass();

		gfx::bufferHandle UniformBufferHandle = GfxContext->CreateBuffer(sizeof(uniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
		gfx::buffer *UniformBuffer = (gfx::buffer*) GfxContext->ResourceManager.Buffers.GetResource(UniformBufferHandle);
		UniformBuffer->CopyData((uint8_t*)&UniformData, sizeof(uniformData), 0);

		//That's the content of a descriptor set
		Uniforms = std::make_shared<gfx::uniformGroup>();
		Uniforms->Initialize();
		Uniforms->Uniforms.push_back({
			"Buffer",
			gfx::uniformType::Buffer,
			0,
			std::shared_ptr<gfx::buffer>(UniformBuffer),
		});
		//Tell the context that we'll be using this uniforms with this pipeline at binding 0
		//It's possible to bind a uniform group to multiple pipelines.
		GfxContext->BindUniformsToPipeline(Uniforms, PipelineHandle, 0);
		
		//Update the bindings
		Uniforms->Update();

		


	}

	void Run()
	{
		while(!Window->ShouldClose())
		{
			Window->PollEvents();
			
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
			
			CommandBuffer->BindUniformGroup(Uniforms, 0);

			CommandBuffer->BindVertexBuffer(VertexBuffer);
			CommandBuffer->DrawTriangles(0, 3); 
			CommandBuffer->EndPass();
			
			GfxContext->EndFrame();

			// Present the rendered frame
			GfxContext->Present();
		}
	}

	void Cleanup()
	{
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
	}

	void OnResize(uint32_t NewWidth, uint32_t NewHeight)
	{
		Width = NewWidth;
		Height = NewHeight;
		GfxContext->OnResize(Width, Height);		
		std::cout << "ON RESIZE " << NewWidth << std::endl;
	}

};

application App;


void OnResizeWindow(app::window &Window, app::v2i NewSize)
{
	App.OnResize(NewSize.x, NewSize.y);
}

int main()
{	
    App.Init();
	App.Run();
	App.Cleanup();
	


	return 0;
}