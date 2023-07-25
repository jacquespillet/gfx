#include <iostream>

#include "Hlgfx/Include/Mesh.h"
#include "Hlgfx/Include/Geometry.h"
#include "Hlgfx/Include/Scene.h"

#include "Hlgfx/Include/Api.h"

#include <imgui.h>


void OnResizeWindow(app::window &Window, app::v2i NewSize);

struct application
{
	std::shared_ptr<hlgfx::context> Context;
	std::shared_ptr<hlgfx::camera> Camera;
	std::shared_ptr<hlgfx::mesh> Mesh;

	//TODO: Take that out
	std::shared_ptr<hlgfx::object3D> NodeClicked = nullptr;

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

		std::shared_ptr<hlgfx::object3D> Obj = std::make_shared<hlgfx::object3D>("Obj0");
		Obj->Transform.SetLocalPosition(hlgfx::v3f(1, 0, 0));
		Obj->Transform.SetLocalScale(hlgfx::v3f(2, 2, 2));
		Mesh->AddObject(Obj);
		std::shared_ptr<hlgfx::object3D> Obj1 = std::make_shared<hlgfx::object3D>("Obj1");
		Obj1->Transform.SetLocalPosition(hlgfx::v3f(2, 0, 0));
		Obj->AddObject(Obj1);
		std::shared_ptr<hlgfx::object3D> Obj2 = std::make_shared<hlgfx::object3D>("Obj2");
		Obj2->Transform.SetLocalPosition(hlgfx::v3f(3, 0, 0));
		Obj2->Transform.SetLocalScale(hlgfx::v3f(2, 2, 2));
		Obj1->AddObject(Obj2);
	}
	
	void Cleanup()
	{
		Context->Cleanup();
	}

	void DrawNodeChildren(std::shared_ptr<hlgfx::object3D> Object)
	{
		static ImGuiTreeNodeFlags BaseFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		for(size_t i=0; i<Object->Children.size(); i++)
		{
			ImGuiTreeNodeFlags NodeFlags = BaseFlags;
			bool NodeOpen = ImGui::TreeNodeEx((void*)(intptr_t)i, NodeFlags, Object->Children[i]->Name, i);
			if(ImGui::IsItemClicked())
			{
				NodeClicked = Object->Children[i];
			}

			//Drag and drop
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
			{
				ImGui::SetDragDropPayload("DND_DEMO_CELL", &Object->Children[i], sizeof(std::shared_ptr<hlgfx::object3D>));    // Set payload to carry the index of our item (could be anything)
				ImGui::Text("Move");
				ImGui::EndDragDropSource();
			}
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_DEMO_CELL"))
				{
					std::shared_ptr<hlgfx::object3D> Payload = *(std::shared_ptr<hlgfx::object3D>*)payload->Data;
					Payload->SetParent(Object->Children[i]);
				}
				ImGui::EndDragDropTarget();
			}			

			if(NodeOpen)
			{
				DrawNodeChildren(Object->Children[i]);
				ImGui::TreePop();
			}
		}
	}

	void RenderTree()
	{
		ImGui::Begin("Scene");

		DrawNodeChildren(Context->Scene);
		if(NodeClicked != nullptr)
		{
			ImGui::InputFloat3("Position", (float*)&NodeClicked->Transform.LocalPosition, 100);
			ImGui::InputFloat3("Rotation", (float*)&NodeClicked->Transform.LocalRotation, 100);
			ImGui::InputFloat3("Scale", (float*)&NodeClicked->Transform.LocalScale, 100);	
		}
		ImGui::End();
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
			 
			RenderTree();

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