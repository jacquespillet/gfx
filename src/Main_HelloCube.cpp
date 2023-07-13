
// #if API == GL
// #include "Gfx/OpenGL/Api.h"
// #elif API == VK
// #elif API == DX
// #include "Gfx/D3D12/Api.h"
// #endif
#include <iostream>

#include "Gfx/Api.h"  
#include "App/App.h"

#include <glm/glm.hpp>
#include <glm/ext.hpp>


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

	struct sceneMatrices
	{
		glm::mat4 ViewMatrix;
		glm::mat4 ProjectionMatrix;
	};

	uniformData UniformData1;
	uniformData UniformData2;
	uniformData UniformData3;
	uniformData UniformData4;
	sceneMatrices SceneMatrices;
	
	gfx::imageHandle TextureHandle1;
	gfx::imageHandle TextureHandle2;

	gfx::bufferHandle UniformBufferHandle1;
	gfx::bufferHandle UniformBufferHandle2;
	gfx::bufferHandle UniformBufferHandle3;
	gfx::bufferHandle UniformBufferHandle4;
	gfx::bufferHandle SceneMatricesBufferHandle;

	uint32_t Width, Height;
	void Init()
	{
		// Create the appropriate graphics API object based on runtime configuration
		Width = 1280;
		Height = 720;

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

		SceneMatrices = 
		{
			glm::lookAt(glm::vec3(1.0f,1.0f,1.0f), glm::vec3(0.0f,0.0f,0.0f), glm::vec3(0.0f,1.0f,0.0f)),
			glm::perspective(glm::radians(60.0f), (float)Width / (float)Height, 0.01f, 100.0f)
		};
		
		gfx::memory::Get()->Init();
    
		
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
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
			0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

			0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
			0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
			0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
			0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
		};			
		
		gfx::vertexStreamData VertexStream1 = {};
		VertexStream1
			.SetSize(sizeof(vertices))
			.SetStride(5 * sizeof(float))
			.SetData(&vertices)
			.SetStreamIndex(0)
			.AddAttribute({sizeof(float), 3, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0})
			.AddAttribute({sizeof(float), 2, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::UV0, 0, 1});
		
		//TODO: This is not ideal....
		VertexBufferHandle = GfxContext->CreateEmptyVertexBuffer();
		gfx::vertexBuffer *VertexBuffer = (gfx::vertexBuffer*) GfxContext->ResourceManager.VertexBuffers.GetResource(VertexBufferHandle);
		VertexBuffer->Init()
					.AddVertexStream(VertexStream1)
					.Create();
		

		gfx::framebufferCreateInfo FramebufferCreateInfo = 
		{
			1024, 1024,
			{gfx::format::R8G8B8A8_UNORM},
			gfx::format::D24_UNORM_S8_UINT
		};
		OffscreenPass = GfxContext->CreateFramebuffer(FramebufferCreateInfo);
		SwapchainPass = GfxContext->GetDefaultRenderPass();
		
		PipelineHandleOffscreen = GfxContext->CreatePipelineFromFile("resources/Shaders/Cube.json", OffscreenPass);
		PipelineHandleSwapchain = GfxContext->CreatePipelineFromFile("resources/Shaders/Cube.json");


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

		SceneMatricesBufferHandle = GfxContext->CreateBuffer(sizeof(sceneMatrices), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
		gfx::buffer *SceneMatricesBuffer = (gfx::buffer*) GfxContext->ResourceManager.Buffers.GetResource(SceneMatricesBufferHandle);
		SceneMatricesBuffer->CopyData((uint8_t*)&SceneMatrices, sizeof(sceneMatrices), 0);

		//That's the content of a descriptor set
		Uniforms = std::make_shared<gfx::uniformGroup>();
		Uniforms->Initialize();
		Uniforms->Uniforms.push_back({
			"Buffer",
			gfx::uniformType::Buffer,
			0,
			UniformBuffer1,
		});
		Uniforms->Uniforms.push_back({
			"Buffer",
			gfx::uniformType::Buffer,
			1,
			UniformBuffer2,
		});
		Uniforms->Uniforms.push_back({
			"Buffer",
			gfx::uniformType::Buffer,
			2,
			UniformBuffer3,
		});
		Uniforms->Uniforms.push_back({
			"Buffer",
			gfx::uniformType::Buffer,
			3,
			UniformBuffer4,
		});
		Uniforms->Uniforms.push_back({
			"Image",
			gfx::uniformType::Texture2d,
			4,
			Texture1,
		});
		Uniforms->Uniforms.push_back({
			"Matrices",
			gfx::uniformType::Buffer,
			5,
			SceneMatricesBuffer,
		});
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
		GfxContext->DestroyBuffer(SceneMatricesBufferHandle);
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
			gfx::buffer *Buffer = (gfx::buffer*)(Uniforms->Uniforms[0].Resource);
			Buffer->CopyData((uint8_t*)&UniformData1, sizeof(uniformData), 0);

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
			CommandBuffer->DrawTriangles(0, 36); 
			CommandBuffer->EndPass();
			
			CommandBuffer->BeginPass(GfxContext->GetSwapchainFramebuffer(), {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});
			CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
			CommandBuffer->SetScissor(0, 0, Width, Height);

			CommandBuffer->BindGraphicsPipeline(PipelineHandleSwapchain);
			
			CommandBuffer->BindUniformGroup(Uniforms, 0);

			CommandBuffer->BindVertexBuffer(VertexBufferHandle);
			CommandBuffer->DrawTriangles(0, 36); 

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