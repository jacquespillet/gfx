#pragma once
#include "App/Window.h"
#include "Gfx/Include/Context.h"
#include "Gfx/Include/ImguiHelper.h"
#include "Types.h"
#include "Camera.h"
#include "Material.h"
#include "Geometry.h"

#include <memory>
#include <unordered_map>
#include <imgui.h>
#include <ImGuizmo.h>
#include <uuid_v4.h>


namespace hlgfx
{
struct shadowsRenderer;
struct mainRenderer;
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
    void OnKeyChanged(app::keyCode KeyCode, b8 KeyDown);
    void Cleanup();

    void Render(std::shared_ptr<camera> Camera);

    std::shared_ptr<camera> CurrentCamera;

    gfx::pipelineCreation GetPipelineCreation(materialFlags::bits Flags, gfx::renderPassHandle = gfx::InvalidHandle);
    gfx::pipelineHandle CreateOrGetPipeline(materialFlags::bits Flags);
    gfx::pipelineHandle GetPipeline(materialFlags::bits Flags);



    b8 ShouldClose();

    ~context();

    u32 Frame = 0;

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
    static const u32 GBufferPipeline = 2;
    static const u32 CompositionPipeline = 4;
    static const u32 RTXReflectionsPipeline = 5;
    std::unordered_map<u32, gfx::pipelineHandle> Pipelines;


    std::unordered_map<materialFlags::bits, gfx::pipelineHandle> AllPipelines;

    static const b8 UseRTX = true;
    // Shadow consts
    static const u32 ShadowMapSize = 1024;
    static const gfx::format ShadowMapFormat = gfx::format::D32_SFLOAT;

    std::string GetUUID();
    struct project
    {
        std::vector<std::shared_ptr<material>> Materials;
        std::vector<std::shared_ptr<indexedGeometryBuffers>> Geometries;
        std::vector<std::shared_ptr<texture>> Textures;
        std::vector<std::shared_ptr<object3D>> Objects;
        std::vector<std::shared_ptr<scene>> Scenes;
    } Project;

    void AddObjectToProject(std::shared_ptr<object3D> Object, u32 Level = 0);
    void RemoveObjectFromProject(std::shared_ptr<object3D> Object);
    void AddMaterialToProject(std::shared_ptr<material> Material);
    void AddTextureToProject(std::shared_ptr<texture> Texture);
    void AddMeshToProject(std::shared_ptr<mesh> Object);
    void AddSceneToProject(std::shared_ptr<scene> Scene);
    void AddGeometryToProject(std::shared_ptr<indexedGeometryBuffers> Geometry);
    void SaveProjectToFile(const char *FolderName);
    void LoadProjectFromFile(const char *FolderName);
    void NewProject();
    void UpdateBindlessDescriptors();


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
    std::shared_ptr<hlgfx::shadowsRenderer> ShadowsRenderer;

    
    // TODO: Make that a struct with the main array, and make that a stack
    void QueueRemoveTextureFromProject(std::shared_ptr<texture> Texture);
    void QueueRemoveMaterialFromProject(std::shared_ptr<material> Material);
    void ProcessTextureDeletionQueue();
    void ProcessMaterialDeletionQueue();
    std::vector<std::shared_ptr<texture>> TextureDeletionQueue;
    std::vector<std::shared_ptr<material>> MaterialDeletionQueue;
    std::vector<u32> TextureFreeIndices;
    std::vector<u32> MaterialFreeIndices;


    // RenderType
    enum class rendererType
    {
        forward,
        deferred
    } RenderType = rendererType::deferred;

    std::shared_ptr<hlgfx::renderer> MainRenderer;
    void SetRenderFlags(materialFlags::bits &Flags);
	
    gfx::imageHandle ShadowMaps;

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