
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

#include <Imgui.h>

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
void OnClickedWindow(app::window &Window, app::mouseButton Button, bool clicked);

struct application
{
	std::shared_ptr<app::window> Window;
	std::shared_ptr<gfx::context> GfxContext;
	std::shared_ptr<gfx::imgui> Imgui;
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
	sceneMatrices SceneMatrices;
	
	gfx::imageHandle TextureHandle1;
	gfx::bufferHandle UniformBufferHandle1;
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
		Window->OnMouseChanged = OnClickedWindow;

		// Initialize the graphics API
		gfx::context::initializeInfo ContextInitialize;
		ContextInitialize.Extensions = Window->GetRequiredExtensions();
		ContextInitialize.ErrorCallback = ErrorCallback;
		ContextInitialize.InfoCallback = InfoCallback;
		ContextInitialize.Debug = true;
		GfxContext = gfx::context::Initialize(ContextInitialize, *Window);

		
		Swapchain = GfxContext->CreateSwapchain(Width, Height);
		
		gfx::imageData CubemapFront = gfx::ImageFromFile("resources/Textures/Cubemap/Front.jpg");
		gfx::imageData CubemapBack = gfx::ImageFromFile("resources/Textures/Cubemap/Back.jpg");
		gfx::imageData CubemapLeft = gfx::ImageFromFile("resources/Textures/Cubemap/Left.jpg");
		gfx::imageData CubemapRight = gfx::ImageFromFile("resources/Textures/Cubemap/Right.jpg");
		gfx::imageData CubemapBottom = gfx::ImageFromFile("resources/Textures/Cubemap/Bottom.jpg");
		gfx::imageData CubemapTop = gfx::ImageFromFile("resources/Textures/Cubemap/Top.jpg");
		gfx::imageCreateInfo ImageCreateInfo = 
		{
			{0.0f,0.0f,0.0f,0.0f},
			gfx::samplerFilter::Linear,
			gfx::samplerFilter::Linear,
			gfx::samplerWrapMode::ClampToBorder,
			gfx::samplerWrapMode::ClampToBorder,
			gfx::samplerWrapMode::ClampToBorder,
			false
		};
		TextureHandle1 = GfxContext->CreateImageCubemap(CubemapLeft, CubemapRight, CubemapTop, CubemapBottom, CubemapBack, CubemapFront, ImageCreateInfo);
		gfx::image *Texture1 = GfxContext->GetImage(TextureHandle1);

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
		
		PipelineHandleSwapchain = GfxContext->CreatePipelineFromFile("resources/Shaders/CubeMap/CubeMap.json");
		PipelineHandleOffscreen = GfxContext->CreatePipelineFromFile("resources/Shaders/CubeMap/CubeMap.json", OffscreenPass);

		Imgui = gfx::imgui::Initialize(GfxContext, Window, SwapchainPass);
  

		UniformBufferHandle1 = GfxContext->CreateBuffer(sizeof(uniformData), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
		gfx::buffer *UniformBuffer1 = GfxContext->GetBuffer(UniformBufferHandle1);
		UniformBuffer1->CopyData((uint8_t*)&UniformData1, sizeof(uniformData), 0);


		SceneMatricesBufferHandle = GfxContext->CreateBuffer(sizeof(sceneMatrices), gfx::bufferUsage::UniformBuffer, gfx::memoryUsage::CpuToGpu);
		gfx::buffer *SceneMatricesBuffer = GfxContext->GetBuffer(SceneMatricesBufferHandle);
		SceneMatricesBuffer->CopyData((uint8_t*)&SceneMatrices, sizeof(sceneMatrices), 0);

		//That's the content of a descriptor set
		Uniforms = std::make_shared<gfx::uniformGroup>();
		Uniforms->Reset()
				.AddUniformBuffer(0, UniformBufferHandle1)
				.AddTexture(4, TextureHandle1)
				.AddUniformBuffer(5, SceneMatricesBufferHandle);
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
		Imgui->Cleanup();

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
		GfxContext->DestroyBuffer(SceneMatricesBufferHandle);
		GfxContext->DestroyPipeline(PipelineHandleSwapchain);
		GfxContext->DestroyPipeline(PipelineHandleOffscreen);
		GfxContext->DestroyFramebuffer(OffscreenPass);
		GfxContext->DestroyVertexBuffer(VertexBufferHandle);
		GfxContext->DestroyImage(TextureHandle1);
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

			CommandBuffer->BeginPass(OffscreenPass, {1.0f, 0.0f, 0.0f, 0.0f}, {1.0f, 0});
			CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
			CommandBuffer->SetScissor(0, 0, Width, Height);

			CommandBuffer->BindGraphicsPipeline(PipelineHandleOffscreen);
			
			CommandBuffer->BindUniformGroup(Uniforms, 0);

			CommandBuffer->BindVertexBuffer(VertexBufferHandle);
			CommandBuffer->DrawArrays(0, 36); 
			CommandBuffer->EndPass();

			CommandBuffer->BeginPass(GfxContext->GetSwapchainFramebuffer(), {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});
			
			Imgui->StartFrame();

			CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
			CommandBuffer->SetScissor(0, 0, Width, Height);

			CommandBuffer->BindGraphicsPipeline(PipelineHandleSwapchain);
			
			bool show=true;
			ImGui::ShowDemoWindow(&show);

			CommandBuffer->BindUniformGroup(Uniforms, 0);

			CommandBuffer->BindVertexBuffer(VertexBufferHandle);
			CommandBuffer->DrawArrays(0, 36); 

			
			CommandBuffer->EndPass();
			
			Imgui->EndFrame(CommandBuffer);
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

	void OnClick(app::mouseButton Button, bool Clicked)
	{
		Imgui->OnClick(Button, Clicked);
	}

};

application App;


void OnResizeWindow(app::window &Window, app::v2i NewSize)
{
	App.OnResize(NewSize.x, NewSize.y);
}

void OnClickedWindow(app::window &Window, app::mouseButton Button, bool Clicked)
{
	App.OnClick(Button, Clicked);
}


int main()
{	
    App.Init();
	App.Run();
	App.Cleanup();
	


	return 0;
}