#include <iostream>

#include "Hlgfx/Include/Mesh.h"
#include "Hlgfx/Include/Geometry.h"
#include "Hlgfx/Include/Scene.h"

#include "Hlgfx/Include/Api.h"


void OnResizeWindow(app::window &Window, app::v2i NewSize);

struct application
{
	std::shared_ptr<hlgfx::context> Context;

	void Init()
	{
		Context = hlgfx::context::Initialize(1280, 720);
		
		std::shared_ptr<hlgfx::mesh> Mesh = std::make_shared<hlgfx::mesh>();
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
			Context->Update();
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