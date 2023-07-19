
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
	gfx::pipelineHandle PipelineHandleCompute;
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
	
	gfx::imageHandle TextureHandle1;
	gfx::imageHandle TextureHandle2;

	gfx::bufferHandle UniformBufferHandle1;
	gfx::bufferHandle UniformBufferHandle2;
	gfx::bufferHandle StorageBufferHandle;

	uint32_t Width, Height;

	uint32_t InstanceCount = 1024;
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
		gfx::image *Texture1 = GfxContext->GetImage(TextureHandle1);
		gfx::image *Texture2 = GfxContext->GetImage(TextureHandle2);

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
		

		gfx::framebufferCreateInfo FramebufferCreateInfo = {};
		FramebufferCreateInfo.SetSize(1024, 1024)
							 .AddColorFormat(gfx::format::R8G8B8A8_UNORM)
							 .SetDepthFormat(gfx::format::D24_UNORM_S8_UINT)
							 .SetClearColor(1, 0, 0, 0);
		OffscreenPass = GfxContext->CreateFramebuffer(FramebufferCreateInfo);
		SwapchainPass = GfxContext->GetDefaultRenderPass();
	
		PipelineHandleCompute = GfxContext->CreatePipelineFromFile("resources/Shaders/Compute/Compute.json");
		PipelineHandleOffscreen = GfxContext->CreatePipelineFromFile("resources/Shaders/Compute/Triangle_Compute.json", OffscreenPass);
		PipelineHandleSwapchain = GfxContext->CreatePipelineFromFile("resources/Shaders/Compute/Triangle_Compute.json");


		UniformBufferHandle1 = GfxContext->CreateBuffer(sizeof(uniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
		gfx::buffer *UniformBuffer1 = GfxContext->GetBuffer(UniformBufferHandle1);
		UniformBuffer1->CopyData((uint8_t*)&UniformData1, sizeof(uniformData), 0);

		UniformBufferHandle2 = GfxContext->CreateBuffer(sizeof(uniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
		gfx::buffer *UniformBuffer2 = GfxContext->GetBuffer(UniformBufferHandle2);
		UniformBuffer2->CopyData((uint8_t*)&UniformData2, sizeof(uniformData), 0);

		std::vector<glm::vec4> InstancePositionsVec(InstanceCount);
		uint32_t i=0;
		for(uint32_t y=0; y<32; y++)
		{
			for(uint32_t x=0; x<32; x++)
			{
				InstancePositionsVec[i] = glm::vec4(
					(((float)x / 32.0f) - 0.5f) * 2.0f,
					(((float)y / 32.0f) - 0.5f) * 2.0f,
					0,0);
				i++;
			}
		}
		StorageBufferHandle = GfxContext->CreateBuffer(InstanceCount * sizeof(glm::vec4), gfx::bufferUsage::StorageBuffer, gfx::memoryUsage::GpuOnly, sizeof(glm::vec4));
		gfx::buffer *StorageBuffer = GfxContext->GetBuffer(StorageBufferHandle);
		GfxContext->CopyDataToBuffer(StorageBufferHandle, InstancePositionsVec.data(), InstanceCount * sizeof(glm::vec4), 0);
		
		//That's the content of a descriptor set  
		Uniforms = std::make_shared<gfx::uniformGroup>();
		Uniforms->Reset()
				.AddUniformBuffer(0, UniformBufferHandle1)
				.AddUniformBuffer(3, UniformBufferHandle2)
				.AddStorageBuffer(2, StorageBufferHandle)
				.AddTexture(4, TextureHandle1);
		
		//Tell the context that we'll be using this uniforms with this pipeline at binding 0
		//It's possible to bind a uniform group to multiple pipelines.
		GfxContext->BindUniformsToPipeline(Uniforms, PipelineHandleOffscreen, 0);
		GfxContext->BindUniformsToPipeline(Uniforms, PipelineHandleSwapchain, 0);
		GfxContext->BindUniformsToPipeline(Uniforms, PipelineHandleCompute, 0);
		
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
		GfxContext->DestroyBuffer(StorageBufferHandle);
		GfxContext->DestroyPipeline(PipelineHandleSwapchain);
		GfxContext->DestroyPipeline(PipelineHandleOffscreen);
		GfxContext->DestroyPipeline(PipelineHandleCompute);
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
			t += 0.1f;
			UniformData1.Color0.r = (cos(t) + 1.0f) * 0.5f;
			Uniforms->UpdateBuffer(0, &UniformData1, sizeof(uniformData), 0);

			UniformData2.Color0.g = (cos(t + 3.14) + 1.0f) * 0.5f;
			Uniforms->UpdateBuffer(1, &UniformData2, sizeof(uniformData), 0);

			Window->PollEvents();
			GfxContext->StartFrame();
			

			// Set up the render state
			std::shared_ptr<gfx::commandBuffer> CommandBuffer = GfxContext->GetCurrentFrameCommandBuffer();

			// Begin recording commands into the command buffer
			CommandBuffer->Begin();

			CommandBuffer->BindComputePipeline(PipelineHandleCompute);
			CommandBuffer->BindUniformGroup(Uniforms, 0);
			CommandBuffer->Dispatch(1, 1, 1);

			CommandBuffer->BeginPass(OffscreenPass, {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});
			CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
			CommandBuffer->SetScissor(0, 0, Width, Height);

			CommandBuffer->BindGraphicsPipeline(PipelineHandleOffscreen);
			
			CommandBuffer->BindUniformGroup(Uniforms, 0);

			CommandBuffer->BindVertexBuffer(VertexBufferHandle);
			CommandBuffer->DrawArrays(0, 3, InstanceCount); 
			CommandBuffer->EndPass();
			
			CommandBuffer->BeginPass(GfxContext->GetSwapchainFramebuffer(), {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});
			CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
			CommandBuffer->SetScissor(0, 0, Width, Height);

			CommandBuffer->BindGraphicsPipeline(PipelineHandleSwapchain);
			
			CommandBuffer->BindUniformGroup(Uniforms, 0);

			CommandBuffer->BindVertexBuffer(VertexBufferHandle);
			CommandBuffer->DrawArrays(0, 3, InstanceCount); 

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