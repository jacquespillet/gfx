#include <iostream>

#include "Hlgfx/Include/Mesh.h"
#include "Hlgfx/Include/Geometry.h"
#include "Hlgfx/Include/Scene.h"

#include "Hlgfx/Include/Api.h"


void OnResizeWindow(app::window &Window, app::v2i NewSize);

struct application
{
	std::shared_ptr<hlgfx::context> Context;
	std::shared_ptr<hlgfx::camera> Camera;
	std::shared_ptr<hlgfx::mesh> Mesh;
	void Init()
	{
		Context = hlgfx::context::Initialize(1280, 720);
		
		Camera = std::make_shared<hlgfx::camera>(60, (float)1280 / (float)720);
		Camera->Transform.SetLocalPosition(hlgfx::v3f(0, 0, 3));
		Camera->RecalculateMatrices();

		std::shared_ptr<hlgfx::orbitCameraController> OrbitControls = std::make_shared<hlgfx::orbitCameraController>(Camera);
		Context->Scene->AddObject(OrbitControls);

		Mesh = std::make_shared<hlgfx::mesh>();
		Mesh->GeometryBuffers = hlgfx::GetTriangleGeometry();
		Mesh->Material = std::make_shared<hlgfx::unlitMaterial>();
		Context->Scene->AddObject(Mesh);
	}
	
	void Cleanup()
	{
		Context->Cleanup();
	}


	void Run()
	{
		float t = 0;
		while(!Context->ShouldClose())
		{
			
			Context->StartFrame();
			
			t += 0.01f;
			float X = cos(t);
			Mesh->Transform.SetLocalPosition(hlgfx::v3f(X, 0, 0));
			 
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
	App.Cleanup();
	return 0;
}