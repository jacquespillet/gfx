
// #if API == GL
// #include "Gfx/OpenGL/Api.h"
// #elif API == VK
// #elif API == DX
// #include "Gfx/D3D12/Api.h"
// #endif
#include "stdint.h"

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
	struct uniformData
	{
		gfx::v4f Color0;
		gfx::v4f Color1;
	};
	uniformData UniformData1;

	std::shared_ptr<app::window> Window;
	uint32_t Width, Height;
	
	std::shared_ptr<gfx::context> GfxContext;
	std::shared_ptr<gfx::swapchain> Swapchain;
	
	gfx::imageHandle TextureHandle1;
	
	gfx::pipelineHandle PipelineHandleSwapchain;

	gfx::vertexBufferHandle VertexBufferHandle;
	gfx::renderPassHandle SwapchainPass;
	
	gfx::bufferHandle UniformBufferHandle1;
	std::shared_ptr<gfx::uniformGroup> Uniforms;


	void Init()
	{
		UniformData1 = 
		{
			gfx::v4f(1,0,0,1),
			gfx::v4f(0,0,0,1)
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

		// e the graphics API
		gfx::context::initializeInfo ContextInitialize;
		ContextInitialize.Extensions = Window->GetRequiredExtensions();
		ContextInitialize.ErrorCallback = ErrorCallback;
		ContextInitialize.InfoCallback = InfoCallback;
		ContextInitialize.Debug = true;
		GfxContext = gfx::context::Initialize(ContextInitialize, *Window);
		Swapchain = GfxContext->CreateSwapchain(Width, Height);
#if 1

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
		gfx::image *Texture1 = GfxContext->GetImage(TextureHandle1);

		PipelineHandleSwapchain = GfxContext->CreatePipelineFromFile("resources/Shaders/Triangle/Triangle.json");
		
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

		SwapchainPass = GfxContext->GetDefaultRenderPass();

		UniformBufferHandle1 = GfxContext->CreateBuffer(sizeof(uniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
		gfx::buffer *UniformBuffer1 = GfxContext->GetBuffer(UniformBufferHandle1);
		UniformBuffer1->CopyData((uint8_t*)&UniformData1, sizeof(uniformData), 0);
		//That's the content of a descriptor set
		Uniforms = std::make_shared<gfx::uniformGroup>();
		Uniforms->Reset()
				.AddUniformBuffer(0, UniformBufferHandle1)
				.AddTexture(4, TextureHandle1);
		
		//Tell the context that we'll be using this uniforms with this pipeline at binding 0
		//It's possible to bind a uniform group to multiple pipelines.
		GfxContext->BindUniformsToPipeline(Uniforms, PipelineHandleSwapchain, 0);
		Uniforms->Update();
#endif
	}
	
	void Cleanup()
	{
		GfxContext->WaitIdle();
#if 1
		DestroyProgramSpecific();
#endif
		GfxContext->DestroySwapchain();
		GfxContext->Cleanup();

		gfx::memory *Memory = gfx::memory::Get();
		Memory->Destroy();
		delete Memory;

		system("pause");
	}

	void DestroyProgramSpecific()
	{
		GfxContext->DestroyImage(TextureHandle1);
		GfxContext->DestroyPipeline(PipelineHandleSwapchain);
		GfxContext->DestroyVertexBuffer(VertexBufferHandle);
		GfxContext->DestroyBuffer(UniformBufferHandle1);
	}


	void Run()
	{
#if 1
		float t = 0;
		while(!Window->ShouldClose())
		{
			Window->PollEvents();
			t += 0.1f;
			UniformData1.Color0.r = (cos(t) + 1.0f) * 0.5f;
			Uniforms->UpdateBuffer(0, &UniformData1, sizeof(uniformData), 0);

			GfxContext->StartFrame();
			

			// Set up the render state
			std::shared_ptr<gfx::commandBuffer> CommandBuffer = GfxContext->GetCurrentFrameCommandBuffer();

			// Begin recording commands into the command buffer
			CommandBuffer->Begin();

			CommandBuffer->BeginPass(GfxContext->GetSwapchainFramebuffer(), {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});
			CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
			CommandBuffer->SetScissor(0, 0, Width, Height);

			CommandBuffer->BindGraphicsPipeline(PipelineHandleSwapchain);
			
			CommandBuffer->BindUniformGroup(Uniforms, 0);

			CommandBuffer->BindVertexBuffer(VertexBufferHandle);
			CommandBuffer->DrawArrays(0, 3); 

			CommandBuffer->EndPass();
			
			GfxContext->EndFrame();

			// Present the rendered frame
			GfxContext->Present();
		}
#endif
	}

	void OnResize(uint32_t NewWidth, uint32_t NewHeight)
	{
		Width = NewWidth;
		Height = NewHeight;
		// GfxContext->OnResize(Width, Height);		
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