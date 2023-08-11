#include <iostream>

#include "Hlgfx/Include/Mesh.h"
#include "Hlgfx/Include/Scene.h"
#include "Hlgfx/Include/Geometry.h"
#include "Hlgfx/Include/Scene.h"
#include "Hlgfx/Include/Renderer.h"
#include "Hlgfx/Include/Material.h"
#include "Hlgfx/Loaders/GLTF.h"

#include "Hlgfx/Include/Api.h"

#include <imgui.h>


void OnResizeWindow(app::window &Window, app::v2i NewSize);

std::shared_ptr<hlgfx::context> Context;

struct application
{
	std::shared_ptr<hlgfx::camera> Camera;
	std::shared_ptr<hlgfx::mesh> Mesh;
	std::shared_ptr<hlgfx::renderer> Renderer;
	
	gfx::framebufferHandle OffscreenFramebufferHandle;
	gfx::pipelineHandle PipelineHandleOffscreen;
	
	void Init(std::string ProjectFile)
	{
		Context = hlgfx::context::Initialize();
		std::shared_ptr<hlgfx::scene> Scene = std::make_shared<hlgfx::scene>();
		Context->AddSceneToProject(Scene);
		
		Camera = std::make_shared<hlgfx::camera>(60, (float)1280 / (float)720);
		Camera->SetLocalPosition(hlgfx::v3f(0, 0, 3));

		gfx::framebufferCreateInfo FramebufferCreateInfo = {};
		FramebufferCreateInfo.SetSize(1024, 1024)
							 .AddColorFormat(gfx::format::R8G8B8A8_UNORM)
							 .SetDepthFormat(gfx::format::D24_UNORM_S8_UINT)
							 .SetClearColor(1, 0, 0, 0);
		OffscreenFramebufferHandle = gfx::context::Get()->CreateFramebuffer(FramebufferCreateInfo);
		PipelineHandleOffscreen = gfx::context::Get()->CreatePipelineFromFile("resources/Hlgfx/Shaders/ShadowMaps/ShadowMaps.json", OffscreenFramebufferHandle);
		
		Renderer = std::make_shared<hlgfx::renderer>();
		Renderer->RenderTarget = OffscreenFramebufferHandle;
		Renderer->OverrideMaterial = std::make_shared<hlgfx::customMaterial>("ShadowMaterial", PipelineHandleOffscreen);
		
		if(ProjectFile != "")
		{
			Context->LoadProjectFromFile(ProjectFile.c_str());
		}
	}

	void Run()
	{
		float t = 0;
		while(!Context->ShouldClose())
		{
			
			Context->StartFrame();
			Context->Update();
			
			Renderer->Render(Context->Scene, Camera);
			
			Context->Render(Camera);
			
			Context->EndFrame();
		}
	}
};

application App;


int main(int argc, char *argv[])
{	
	std::string ProjectFolder="";
	if(argc>1)
		ProjectFolder = std::string(argv[1]);
    App.Init(ProjectFolder);
	App.Run();
	return 0;
}