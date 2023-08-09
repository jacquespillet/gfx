#pragma once
#include "Types.h"
#include <imgui.h>
#include <ImGuizmo.h>
#include <memory>
namespace hlgfx
{
struct object3D;
struct material;
struct context;
struct texture;
struct scene;
struct indexedGeometryBuffers;
struct contextGUI
{
    contextGUI(context *Context);
    void StartFrame();
   
    void DrawGUI();
    void DrawObjectMenu();
    void AddObjectMenu();
    void DrawGuizmoGUI();
    void DrawMainMenuBar();
    void DrawAssetsWindow();
   
    context *Context;
    
    b8 ShowAssetsWindow = false;
    
    //Inputs
    b8 IsInteractingGUI = false;
    b8 IsRenaming = false;

    ImGuizmo::OPERATION CurrentGizmoOperation = (ImGuizmo::TRANSLATE);
    ImGuizmo::MODE CurrentGizmoMode = (ImGuizmo::WORLD);    

    std::shared_ptr<material> SelectedMaterial = nullptr;
    std::shared_ptr<indexedGeometryBuffers> SelectedIndexedGeometryBuffers = nullptr;
    std::shared_ptr<scene> SelectedScene = nullptr;
    std::shared_ptr<texture> SelectedTexture = nullptr;
    std::shared_ptr<object3D> SelectedObject3D = nullptr;
        
};

}