#include <iostream>

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

	
	void Init(std::string ProjectFile)
	{
		Context = hlgfx::context::Initialize();
		std::shared_ptr<hlgfx::scene> Scene = std::make_shared<hlgfx::scene>();
		Context->AddSceneToProject(Scene);
		
		Camera = std::make_shared<hlgfx::camera>(60, (float)Context->Width / (float)Context->Height, 0.01f, 100.0f);
		Camera->SetLocalPosition(hlgfx::v3f(0, 0, 3));

		Context->CurrentCamera = Camera;
		
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