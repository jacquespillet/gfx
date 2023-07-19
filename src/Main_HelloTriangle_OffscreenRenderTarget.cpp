
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
	gfx::framebufferHandle OffscreenFramebufferHandle;
	gfx::pipelineHandle PipelineHandleOffscreen;
	gfx::pipelineHandle PipelineHandleSwapchain;
	gfx::vertexBufferHandle VertexBufferHandle;
	std::shared_ptr<gfx::swapchain> Swapchain;
	std::shared_ptr<gfx::uniformGroup> UniformsOffscreen;
	std::shared_ptr<gfx::uniformGroup> UniformsFinalRender;


	gfx::imageHandle TextureHandle1;

	uint32_t Width, Height;
	void Init()
	{	
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
		ContextInitialize.Debug = true;
		GfxContext = gfx::context::Initialize(ContextInitialize, *Window);

		Swapchain = GfxContext->CreateSwapchain(Width, Height);
		
		gfx::imageData ImageData = gfx::ImageFromFile("resources/Textures/Debug.jpg");
		gfx::imageCreateInfo ImageCreateInfo = 
		{
			{0.0f,0.0f,0.0f,0.0f},
			gfx::samplerFilter::Linear,
			gfx::samplerFilter::Linear,
			gfx::samplerWrapMode::ClampToBorder,
			gfx::samplerWrapMode::ClampToBorder,
			gfx::samplerWrapMode::ClampToBorder,
			true
		};
		TextureHandle1 = GfxContext->CreateImage(ImageData, ImageCreateInfo);
		gfx::image *Texture1 = (gfx::image*) GfxContext->ResourceManager.Images.GetResource(TextureHandle1);


		float vertices[] =
		{
			0.0f, 0.25f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
			0.25f, -0.25f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
			-0.25f, -0.25f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f
		};
		
		gfx::vertexStreamData VertexStream1 = {};
		VertexStream1
			.SetSize(sizeof(vertices))
			.SetStride(7 * sizeof(float))
			.SetData(&vertices)
			.SetStreamIndex(0)
			.AddAttribute({sizeof(float), 3, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0})
			.AddAttribute({sizeof(float), 4, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::COLOR, 0, 1});
		
		gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
		VertexBufferCreateInfo.Init()
							  .AddVertexStream(VertexStream1);
		VertexBufferHandle = GfxContext->CreateVertexBuffer(VertexBufferCreateInfo);

		gfx::framebufferCreateInfo FramebufferCreateInfo = 
		{
			1024, 1024,
			{gfx::format::R8G8B8A8_UNORM},
			gfx::format::D24_UNORM_S8_UINT
		};
		OffscreenFramebufferHandle = GfxContext->CreateFramebuffer(FramebufferCreateInfo);
		gfx::framebuffer *OffscreenFramebuffer = (gfx::framebuffer*) GfxContext->ResourceManager.Framebuffers.GetResource(OffscreenFramebufferHandle);
		
		PipelineHandleOffscreen = GfxContext->CreatePipelineFromFile("resources/Shaders/OffscreenRenderTarget/TriangleOffscreen.json", OffscreenFramebufferHandle);
		PipelineHandleSwapchain = GfxContext->CreatePipelineFromFile("resources/Shaders/OffscreenRenderTarget/Triangle.json");
  
	
		//That's the content of a descriptor set
		UniformsOffscreen = std::make_shared<gfx::uniformGroup>();
		UniformsOffscreen->Initialize();
		UniformsOffscreen->Uniforms.push_back({
			"Image",
			gfx::uniformType::Texture2d,
			4,
			Texture1,
		});
		GfxContext->BindUniformsToPipeline(UniformsOffscreen, PipelineHandleOffscreen, 0);
		UniformsOffscreen->Update();

		UniformsFinalRender = std::make_shared<gfx::uniformGroup>();
		UniformsFinalRender->Initialize();
		UniformsFinalRender->Uniforms.push_back({
			"Image",
			gfx::uniformType::FramebufferRenderTarget,
			4,
			OffscreenFramebuffer,
			0
		});
		GfxContext->BindUniformsToPipeline(UniformsFinalRender, PipelineHandleSwapchain, 1);
		UniformsFinalRender->Update();
		
	}
	
	void Cleanup()
	{
		GfxContext->WaitIdle();

		DestroyProgramSpecific();

		GfxContext->DestroySwapchain();
		GfxContext->Cleanup();

		gfx::memory *Memory = gfx::memory::Get();
		Memory->Destroy();
		delete Memory;

		system("pause");
	}

	void DestroyProgramSpecific()
	{
		GfxContext->DestroyPipeline(PipelineHandleSwapchain);
		GfxContext->DestroyPipeline(PipelineHandleOffscreen);
		GfxContext->DestroyFramebuffer(OffscreenFramebufferHandle);
		GfxContext->DestroyVertexBuffer(VertexBufferHandle);
		GfxContext->DestroyImage(TextureHandle1);
	}


	void Run()
	{
		float t = 0;
		while(!Window->ShouldClose())
		{
			Window->PollEvents();
			GfxContext->StartFrame();
			

			// Set up the render state
			std::shared_ptr<gfx::commandBuffer> CommandBuffer = GfxContext->GetCurrentFrameCommandBuffer();

			// Begin recording commands into the command buffer
			CommandBuffer->Begin();

			CommandBuffer->SetViewport(0.0f, 0.0f, (float)1024, (float)1024);
			CommandBuffer->SetScissor(0, 0, 1024, 1024);
			CommandBuffer->BeginPass(OffscreenFramebufferHandle, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0});

			CommandBuffer->BindGraphicsPipeline(PipelineHandleOffscreen);
			
			CommandBuffer->BindUniformGroup(UniformsOffscreen, 0);

			CommandBuffer->BindVertexBuffer(VertexBufferHandle);
			CommandBuffer->DrawArrays(0, 3); 
			CommandBuffer->EndPass();
			
			CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
			CommandBuffer->SetScissor(0, 0, Width, Height);
			CommandBuffer->BeginPass(GfxContext->GetSwapchainFramebuffer(), {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});

			CommandBuffer->BindGraphicsPipeline(PipelineHandleSwapchain);
			
			CommandBuffer->BindUniformGroup(UniformsFinalRender, 0);

			CommandBuffer->BindVertexBuffer(VertexBufferHandle);
			CommandBuffer->DrawArrays(0, 3); 

			CommandBuffer->EndPass();
			
			GfxContext->EndFrame();

			// Present the rendered frame
			GfxContext->Present();
		}
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