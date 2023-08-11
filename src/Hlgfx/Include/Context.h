#pragma once
#include "App/Window.h"
#include "Gfx/Include/Context.h"
#include "Gfx/Include/Imgui.h"
#include "Types.h"
#include "Camera.h"
#include "Material.h"
#include "Geometry.h"

#include <memory>
#include <unordered_map>
#include <imgui.h>
#include <ImGuizmo.h>
#include <uuid_v4/uuid_v4.h>


namespace hlgfx
{
struct renderer;
struct scene;  
struct mesh;  
struct contextGUI;  
void OnResizeWindow(app::window &Window, app::v2i NewSize);
void OnClickedWindow(app::window &Window, app::mouseButton Button, bool Clicked);
void OnMousePositionChangedWindow(app::window &Window, f64 PosX, f64 PosY);
void OnMouseWheelChangedWindow(app::window &Window, f64 OffsetX, f64 OffsetY);

struct context
{
    static UUIDv4::UUIDGenerator<std::mt19937_64> UUIDGenerator;
    static std::shared_ptr<context> Singleton;

    static context *Get();
    static std::shared_ptr<context> Initialize(u32 Width=0, u32 Height=0);

    void StartFrame();
    void EndFrame();
    void Update();
    void OnResize(u32 Width, u32 Height);
    void OnMouseClicked(app::mouseButton Button, bool Clicked);
    void OnMousePositionChanged(f64 NewPosX, f64 NewPosY);
    void OnMouseWheelChanged(f64 OffsetX, f64 OffsetY);
    void Cleanup();

    void Render(std::shared_ptr<camera> Camera);

    gfx::pipelineCreation GetPipelineCreation(materialFlags::bits Flags);
    gfx::pipelineHandle CreateOrGetPipeline(materialFlags::bits Flags);
    gfx::pipelineHandle GetPipeline(materialFlags::bits Flags);



    b8 ShouldClose();

    ~context();

    u32 Width, Height;
    std::shared_ptr<app::window> Window;
	std::shared_ptr<gfx::context> GfxContext;
	gfx::renderPassHandle SwapchainPass;
	gfx::pipelineHandle PipelineHandleSwapchain;
	std::shared_ptr<gfx::swapchain> Swapchain;    

    //That's not optimal : 
    //We create these global pipelines only so we can get their descriptor layouts
    //So we can call BindUniformsToPipeline() on them
    //Then each material instantiates another pipeline
    static const u32 PBRPipeline = 0;
    static const u32 ShadowsPipeline = 1;
    std::unordered_map<u32, gfx::pipelineHandle> Pipelines;

    std::unordered_map<materialFlags::bits, gfx::pipelineHandle> AllPipelines;
    std::string GetUUID();
    struct project
    {
        std::unordered_map<std::string, std::shared_ptr<material>> Materials;
        std::unordered_map<std::string, std::shared_ptr<indexedGeometryBuffers>> Geometries;
        std::unordered_map<std::string, std::shared_ptr<texture>> Textures;
        std::unordered_map<std::string, std::shared_ptr<object3D>> Objects;
        std::unordered_map<std::string, std::shared_ptr<scene>> Scenes;
    } Project;
    void AddObjectToProject(std::shared_ptr<object3D> Object, u32 Level = 0);
    void RemoveObjectFromProject(std::shared_ptr<object3D> Object);
    void AddMaterialToProject(std::shared_ptr<material> Material);
    void RemoveMaterialFromProject(std::shared_ptr<material> Material);
    void AddTextureToProject(std::shared_ptr<texture> Texture);
    void RemoveTextureFromProject(std::shared_ptr<texture> Texture);
    void AddMeshToProject(std::shared_ptr<mesh> Object);
    void AddSceneToProject(std::shared_ptr<scene> Scene);
    void AddGeometryToProject(std::shared_ptr<indexedGeometryBuffers> Geometry);
    void SaveProjectToFile(const char *FolderName);
    void LoadProjectFromFile(const char *FolderName);
    void NewProject();
    std::shared_ptr<scene> Scene = nullptr;

    std::shared_ptr<gfx::imgui> Imgui;
    std::shared_ptr<contextGUI> GUI;
	
    std::shared_ptr<pbrMaterial> NoMaterial;

    std::shared_ptr<indexedGeometryBuffers> Quad;
    std::shared_ptr<indexedGeometryBuffers> Cube;
    std::shared_ptr<indexedGeometryBuffers> Sphere;
    std::shared_ptr<indexedGeometryBuffers> Cone;
    std::shared_ptr<indexedGeometryBuffers> Capsule;
    std::shared_ptr<indexedGeometryBuffers> Cylinder;

    //Shadow maps
    std::shared_ptr<hlgfx::renderer> ShadowsRenderer;
	gfx::framebufferHandle ShadowsFramebuffer;
	gfx::pipelineHandle PipelineHandleOffscreen;


    //Inputs
    b8 MouseClicked=false;
    b8 MouseReleased =false;
    app::mouseButton ButtonClicked;
    v2i MouseDelta;
    v2i MousePosition;
    b8 MouseMoved=false;
    b8 LeftButtonPressed=false;
    b8 RightButtonPressed=false;
    b8 MouseWheelChanged=false;
    f64 MouseWheelX = 0;
    f64 MouseWheelY = 0;
    b8 CtrlPressed = false;

    
};

}