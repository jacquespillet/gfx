#include <iostream>

#include "Hlgfx/Include/Mesh.h"
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

	void Init()
	{
		Context = hlgfx::context::Initialize();
		
		Camera = std::make_shared<hlgfx::camera>(60, (float)1280 / (float)720);
		Camera->SetLocalPosition(hlgfx::v3f(0, 0, 3));
		
		std::shared_ptr<hlgfx::object3D> Mesh = hlgfx::loaders::gltf::Load("C:/Users/jacqu/Documents/Boulot/Models/2.0/FlightHelmet/glTF/FlightHelmet.gltf");
		Context->AddObjectToProject(Mesh);

		Context->Scene->AddObject(Mesh->Clone());
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


int main()
{	
    App.Init();
	App.Run();
	return 0;
}