
// #if API == GL
// #include "Gfx/OpenGL/Api.h"
// #elif API == VK
// #elif API == DX
// #include "Gfx/D3D12/Api.h"
// #endif
#include <iostream>

#include "Gfx/Api.h"  
#include "App/App.h"

#define MULTISTREAM 0


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
	gfx::renderPassHandle OffscreenPass;
	gfx::pipelineHandle PipelineHandleOffscreen;
	gfx::pipelineHandle PipelineHandleSwapchain;
	gfx::vertexBufferHandle VertexBufferHandle;
	std::shared_ptr<gfx::swapchain> Swapchain;
	std::shared_ptr<gfx::uniformGroup> Uniforms;


	struct uniformData
	{
		gfx::v4f Color0;
		gfx::v4f Color1;
	};

	uniformData UniformData1;
	uniformData UniformData2;
	uniformData UniformData3;
	uniformData UniformData4;
	
	gfx::imageHandle TextureHandle1;
	gfx::imageHandle TextureHandle2;

	gfx::bufferHandle UniformBufferHandle1;
	gfx::bufferHandle UniformBufferHandle2;
	gfx::bufferHandle UniformBufferHandle3;
	gfx::bufferHandle UniformBufferHandle4;

	uint32_t Width, Height;
	void Init()
	{
		UniformData1 = 
		{
			gfx::v4f(1,0,0,1),
			gfx::v4f(0,0,0,1)
		};
		UniformData2 = 
		{
			gfx::v4f(0,1,0,1),
			gfx::v4f(0,0,0,1)
		};
		UniformData3 = 
		{
			gfx::v4f(0,0,1,1),
			gfx::v4f(0,0,0,1)
		};
		UniformData4 = 
		{
			gfx::v4f(1,0,1,1),
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
		TextureHandle2 = GfxContext->CreateImage(ImageData, ImageCreateInfo);
		gfx::image *Texture1 = (gfx::image*) GfxContext->ResourceManager.Images.GetResource(TextureHandle1);
		gfx::image *Texture2 = (gfx::image*) GfxContext->ResourceManager.Images.GetResource(TextureHandle2);

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
		
		float InstancePositions[] = 
		{
			0.0f, 0.0f,
			0.1f, 0.1f,
			0.2f, 0.2f,
			0.3f, 0.3f,
			0.4f, 0.4f,
			0.5f, 0.5f,
			0.6f, 0.6f,
			0.7f, 0.7f,
			0.8f, 0.8f,
			0.9f, 0.9f,
		};

		gfx::vertexStreamData VertexStream2 = {};
		VertexStream2
			.SetSize(sizeof(InstancePositions))
			.SetStride(2 * sizeof(float))
			.SetData(&InstancePositions)
			.SetStreamIndex(1)
			.SetInputRate(gfx::vertexInputRate::PerInstance)
			.AddAttribute({sizeof(float), 2, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 1, 2});
		

		gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
		VertexBufferCreateInfo.Init()
							  .AddVertexStream(VertexStream1)
							  .AddVertexStream(VertexStream2);
		VertexBufferHandle = GfxContext->CreateVertexBuffer(VertexBufferCreateInfo);
		

		gfx::framebufferCreateInfo FramebufferCreateInfo = 
		{
			1024, 1024,
			{gfx::format::R8G8B8A8_UNORM},
			gfx::format::D24_UNORM_S8_UINT
		};
		OffscreenPass = GfxContext->CreateFramebuffer(FramebufferCreateInfo);
		SwapchainPass = GfxContext->GetDefaultRenderPass();
	
		PipelineHandleOffscreen = GfxContext->CreatePipelineFromFile("resources/Shaders/Instancing/Triangle_Instanced.json", OffscreenPass);
		PipelineHandleSwapchain = GfxContext->CreatePipelineFromFile("resources/Shaders/Instancing/Triangle_Instanced.json");


		UniformBufferHandle1 = GfxContext->CreateBuffer(sizeof(uniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
		gfx::buffer *UniformBuffer1 = (gfx::buffer*) GfxContext->ResourceManager.Buffers.GetResource(UniformBufferHandle1);
		UniformBuffer1->CopyData((uint8_t*)&UniformData1, sizeof(uniformData), 0);

		UniformBufferHandle2 = GfxContext->CreateBuffer(sizeof(uniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
		gfx::buffer *UniformBuffer2 = (gfx::buffer*) GfxContext->ResourceManager.Buffers.GetResource(UniformBufferHandle2);
		UniformBuffer2->CopyData((uint8_t*)&UniformData2, sizeof(uniformData), 0);
  
		UniformBufferHandle3 = GfxContext->CreateBuffer(sizeof(uniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
		gfx::buffer *UniformBuffer3 = (gfx::buffer*) GfxContext->ResourceManager.Buffers.GetResource(UniformBufferHandle3);
		UniformBuffer3->CopyData((uint8_t*)&UniformData3, sizeof(uniformData), 0);

		UniformBufferHandle4 = GfxContext->CreateBuffer(sizeof(uniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
		gfx::buffer *UniformBuffer4 = (gfx::buffer*) GfxContext->ResourceManager.Buffers.GetResource(UniformBufferHandle4);
		UniformBuffer4->CopyData((uint8_t*)&UniformData4, sizeof(uniformData), 0);

		//That's the content of a descriptor set  
		Uniforms = std::make_shared<gfx::uniformGroup>();
		Uniforms->Reset()
				.AddUniformBuffer(0, UniformBufferHandle1)
				.AddUniformBuffer(1, UniformBufferHandle2)
				.AddUniformBuffer(2, UniformBufferHandle3)
				.AddUniformBuffer(3, UniformBufferHandle4)
				.AddTexture(4, TextureHandle1);
		
		//Tell the context that we'll be using this uniforms with this pipeline at binding 0
		//It's possible to bind a uniform group to multiple pipelines.
		GfxContext->BindUniformsToPipeline(Uniforms, PipelineHandleOffscreen, 0);
		GfxContext->BindUniformsToPipeline(Uniforms, PipelineHandleSwapchain, 0);
		
		//Update the bindings
		Uniforms->Update();
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
		GfxContext->DestroyBuffer(UniformBufferHandle1);
		GfxContext->DestroyBuffer(UniformBufferHandle2);
		GfxContext->DestroyBuffer(UniformBufferHandle3);
		GfxContext->DestroyBuffer(UniformBufferHandle4);
		GfxContext->DestroyPipeline(PipelineHandleSwapchain);
		GfxContext->DestroyPipeline(PipelineHandleOffscreen);
		GfxContext->DestroyFramebuffer(OffscreenPass);
		GfxContext->DestroyVertexBuffer(VertexBufferHandle);
		GfxContext->DestroyImage(TextureHandle1);
		GfxContext->DestroyImage(TextureHandle2);
	}


	void Run()
	{
		float t = 0;
		while(!Window->ShouldClose())
		{
			t += 0.01f;
			UniformData1.Color0.r = (cos(t) + 1.0f) * 0.5f;
			Uniforms->UpdateBuffer(0, &UniformData1, sizeof(uniformData), 0);

			Window->PollEvents();
			GfxContext->StartFrame();
			

			// Set up the render state
			std::shared_ptr<gfx::commandBuffer> CommandBuffer = GfxContext->GetCurrentFrameCommandBuffer();

			// Begin recording commands into the command buffer
			CommandBuffer->Begin();

			CommandBuffer->BeginPass(OffscreenPass, {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});
			CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
			CommandBuffer->SetScissor(0, 0, Width, Height);

			CommandBuffer->BindGraphicsPipeline(PipelineHandleOffscreen);
			
			CommandBuffer->BindUniformGroup(Uniforms, 0);

			CommandBuffer->BindVertexBuffer(VertexBufferHandle);
			CommandBuffer->DrawArrays(0, 3, 10); 
			CommandBuffer->EndPass();
			
			CommandBuffer->BeginPass(GfxContext->GetSwapchainFramebuffer(), {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});
			CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
			CommandBuffer->SetScissor(0, 0, Width, Height);

			CommandBuffer->BindGraphicsPipeline(PipelineHandleSwapchain);
			
			CommandBuffer->BindUniformGroup(Uniforms, 0);

			CommandBuffer->BindVertexBuffer(VertexBufferHandle);
			CommandBuffer->DrawArrays(0, 3, 10); 

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