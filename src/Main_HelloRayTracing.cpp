
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
#include <glm/gtc/matrix_transform.hpp>


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
	uint32_t Width, Height;
	
	std::shared_ptr<gfx::context> GfxContext;
	std::shared_ptr<gfx::swapchain> Swapchain;
	
	gfx::pipelineHandle PipelineHandleSwapchain;
	gfx::pipelineHandle PipelineHandleRTX;

	gfx::vertexBufferHandle QuadVertexBufferHandle;
	gfx::vertexBufferHandle TriangleVertexBufferHandle;

	gfx::renderPassHandle SwapchainPass;
	
	std::shared_ptr<gfx::uniformGroup> QuadUniforms;


	std::shared_ptr<gfx::uniformGroup> UniformsRTX;
	gfx::accelerationStructureHandle BLAS;
	gfx::accelerationStructureHandle TLAS;
	gfx::imageHandle OutputImage;

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

		// e the graphics API
		gfx::context::initializeInfo ContextInitialize;
		ContextInitialize.Extensions = Window->GetRequiredExtensions();
		ContextInitialize.ErrorCallback = ErrorCallback;
		ContextInitialize.InfoCallback = InfoCallback;
		ContextInitialize.Debug = true;
		ContextInitialize.EnableRTX = true;
		GfxContext = gfx::context::Initialize(ContextInitialize, *Window);
		Swapchain = GfxContext->CreateSwapchain(Width, Height);


		{
			float QuadVertices[] =
			{
				-1.0f,  -1.0f, 0.0f,          0.0f, 0.0f,
				 1.0f,   1.0f, 0.0f,          1.0f, 1.0f,
				-1.0f,   1.0f, 0.0f,          0.0f, 1.0f,
				
				-1.0f,  -1.0f, 0.0f,          0.0f, 0.0f,
				 1.0f,  -1.0f, 0.0f,          1.0f, 0.0f,
				 1.0f,   1.0f, 0.0f,          1.0f, 1.0f,
			};
			gfx::vertexStreamData VertexStream1 = {};
			VertexStream1
				.SetSize(sizeof(QuadVertices))
				.SetStride(7 * sizeof(float))
				.SetData(&QuadVertices)
				.SetStreamIndex(0)
				.AddAttribute({sizeof(float), 3, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0})
				.AddAttribute({sizeof(float), 2, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::UV0, 0, 1});
			gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
			VertexBufferCreateInfo.Init()
								.AddVertexStream(VertexStream1);
			QuadVertexBufferHandle = GfxContext->CreateVertexBuffer(VertexBufferCreateInfo);
		}		
 
		SwapchainPass = GfxContext->GetDefaultRenderPass();

		PipelineHandleSwapchain = GfxContext->CreatePipelineFromFile("resources/Shaders/RTX/Quad.json");
		
		// RTX


		{
			float TriangleVertices[] =
			{
				0.0f, 0.25f, 0.0f,
				0.25f, -0.25f, 0.0f,
				-0.25f, -0.25f, 0.0f,
			};
			gfx::vertexStreamData VertexStream1 = {};
			VertexStream1
				.SetSize(sizeof(TriangleVertices))
				.SetStride(7 * sizeof(float))
				.SetData(&TriangleVertices)
				.SetStreamIndex(0)
				.AddAttribute({sizeof(float), 3, gfx::vertexAttributeType::Float, false, gfx::attributeSemantic::POSITION, 0, 0});
			gfx::vertexBufferCreateInfo VertexBufferCreateInfo = {};
			VertexBufferCreateInfo.Init()
								.AddVertexStream(VertexStream1);
			TriangleVertexBufferHandle = GfxContext->CreateVertexBuffer(VertexBufferCreateInfo);
		}		

		// Create BLAS
		gfx::vertexBuffer *VertexBuffer = GfxContext->GetVertexBuffer(TriangleVertexBufferHandle);
		// Arguments are : (Number of vertices), (Stride), (Format of the vertex position), (Buffer Handle of the vertex buffer).
		// Optional arguments for indexed geometry : (IndexType), (Num Triangles), (Index Buffer Handle), (Offset of the position in the vertex struct)
		BLAS = GfxContext->CreateBLAccelerationStructure(3, 3 * sizeof(float), gfx::format::R32G32B32_SFLOAT, VertexBuffer->VertexStreams[0].Buffer);
		
		// // Create TLAS
		
		std::vector<glm::mat4> Transforms = {
			glm::mat4(1),
			glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, 0.0f, 0.0f)),
			glm::translate(glm::mat4(1.0f), glm::vec3( 0.5f, 0.0f, 0.0f)),
			};
		std::vector<gfx::accelerationStructureHandle> AccelerationStructures = {BLAS};
		std::vector<int> Instances = {0, 0, 0}; 

		TLAS = GfxContext->CreateTLAccelerationStructure(Transforms, AccelerationStructures, Instances);
		
		PipelineHandleRTX = GfxContext->CreatePipelineFromFile("resources/Shaders/RTX/TriangleRTX.json");


		OutputImage = GfxContext->CreateImage(Width, Height, gfx::format::R8G8B8A8_UNORM, gfx::imageUsage::STORAGE, gfx::memoryUsage::GpuOnly, nullptr);		

		UniformsRTX = std::make_shared<gfx::uniformGroup>();
		UniformsRTX->Reset()
				.AddAccelerationStructure(0, TLAS)
				.AddStorageImage(1, OutputImage); 
		 
		//Tell the context that we'll be using this uniformsRTX with this pipeline at binding 0
		//It's possible to bind a uniform group to multiple pipelines.
		GfxContext->BindUniformsToPipeline(UniformsRTX, PipelineHandleRTX, 0);
		UniformsRTX->Update();		


		//That's the content of a descriptor set
		QuadUniforms = std::make_shared<gfx::uniformGroup>();
		QuadUniforms->Reset()
				.AddStorageImage(0, OutputImage);
		
		GfxContext->BindUniformsToPipeline(QuadUniforms, PipelineHandleSwapchain, 0);
		QuadUniforms->Update();
		
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
		GfxContext->DestroyVertexBuffer(QuadVertexBufferHandle);
		GfxContext->DestroyVertexBuffer(TriangleVertexBufferHandle);
		
		GfxContext->DestroyAccelerationStructure(BLAS);
		GfxContext->DestroyAccelerationStructure(TLAS);
		GfxContext->DestroyPipeline(PipelineHandleRTX);
		GfxContext->DestroyImage(OutputImage);

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
			CommandBuffer->BindRayTracingPipeline(PipelineHandleRTX);
			CommandBuffer->BindUniformGroup(UniformsRTX, 0);

			CommandBuffer->RayTrace(Width, Height, 1, 0, 1, 2);

			CommandBuffer->BeginPass(GfxContext->GetSwapchainFramebuffer(), {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});
			CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
			CommandBuffer->SetScissor(0, 0, Width, Height);

			CommandBuffer->BindGraphicsPipeline(PipelineHandleSwapchain);
			
			CommandBuffer->BindUniformGroup(QuadUniforms, 0);

			CommandBuffer->BindVertexBuffer(QuadVertexBufferHandle);
			CommandBuffer->DrawArrays(0, 6); 


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