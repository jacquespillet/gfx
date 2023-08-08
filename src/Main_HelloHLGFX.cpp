#include <iostream>

#include "Hlgfx/Include/Mesh.h"
#include "Hlgfx/Include/Scene.h"
#include "Hlgfx/Include/Geometry.h"
#include "Hlgfx/Include/Scene.h"
#include "Hlgfx/Loaders/GLTF.h"

#include "Hlgfx/Include/Api.h"

#include <imgui.h>


void OnResizeWindow(app::window &Window, app::v2i NewSize);

std::shared_ptr<hlgfx::context> Context;

struct application
{
	std::shared_ptr<hlgfx::camera> Camera;
	std::shared_ptr<hlgfx::mesh> Mesh;

	void Init(std::string ProjectFile)
	{
		Context = hlgfx::context::Initialize();
		std::shared_ptr<hlgfx::scene> Scene = std::make_shared<hlgfx::scene>();
		Context->AddSceneToProject(Scene);
		
		Camera = std::make_shared<hlgfx::camera>(60, (float)1280 / (float)720);
		Camera->SetLocalPosition(hlgfx::v3f(0, 0, 3));
		
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
			
			// t += 0.01f;
			// float X = cos(t);
			// Mesh->Transform.SetLocalPosition(hlgfx::v3f(X, 0, 0));


			Context->Update(Camera);
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