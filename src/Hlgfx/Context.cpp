#include "Include/Context.h"
#include "Include/Scene.h"
#include "Include/CameraController.h"
#include "Include/Material.h"
#include "Include/Util.h"
#include "Include/Bindings.h"
#include "Include/Geometry.h"
#include "Include/Renderer.h"
#include "Include/GUI.h"
#include "Loaders/GLTF.h"
#include "Gfx/Include/CommandBuffer.h"
#include "Gfx/Common/Util.h"
#include <iostream>
#include <filesystem>
#include <queue>
#include <glm/ext.hpp>

namespace hlgfx
{
UUIDv4::UUIDGenerator<std::mt19937_64> context::UUIDGenerator = UUIDv4::UUIDGenerator<std::mt19937_64>();

void WindowErrorCallback(const std::string &errorMessage)
{
    std::cout << "Window Error : " << errorMessage << std::endl;
}

void ErrorCallback(const std::string &message)
{
    std::cout << " Error : " << message << std::endl;
}

void InfoCallback(const std::string &message)
{
    std::cout << " Info : " << message << std::endl;
}

void OnResizeWindow(app::window &Window, app::v2i NewSize)
{
    context::Get()->OnResize(NewSize.x, NewSize.y);
}

void OnClickedWindow(app::window &Window, app::mouseButton Button, bool Clicked)
{
    context::Get()->OnMouseClicked(Button, Clicked);
}

void OnMousePositionChangedWindow(app::window &Window, f64 PosX, f64 PosY)
{
    context::Get()->OnMousePositionChanged(PosX, PosY);
}

void OnMouseWheelChangedWindow(app::window &Window, f64 OffsetX, f64 OffsetY)
{
    context::Get()->OnMouseWheelChanged(OffsetX, OffsetY);
}

void OnKeyChangedWindow(app::window &Window, app::keyCode KeyCode, b8 KeyDown)
{
    context::Get()->OnKeyChanged(KeyCode, KeyDown);
}

std::shared_ptr<context> context::Singleton = {};

b8 context::ShouldClose()
{
    return this->Window->ShouldClose();
}

context *context::Get()
{
    return Singleton.get();
}

std::shared_ptr<context> context::Initialize(u32 Width, u32 Height)
{
    if(Singleton==nullptr){
        Singleton = std::make_shared<context>();
    }



    gfx::memory::Get()->Init();
	app::windowCreateOptions WindowCreateOptions = {};
    WindowCreateOptions.Size = app::v2f(Width, Height);
    WindowCreateOptions.ErrorCallback = WindowErrorCallback;
#if GFX_API == GFX_VK
    WindowCreateOptions.VersionMajor = 1;
    WindowCreateOptions.VersionMinor = 0;
#elif GFX_API == GFX_GL
    WindowCreateOptions.VersionMajor = 4;
    WindowCreateOptions.VersionMinor = 5;
#endif
    Singleton->Window = std::make_shared<app::window>(WindowCreateOptions);
    Singleton->Window->OnResize = OnResizeWindow;
    Singleton->Window->OnMouseChanged = OnClickedWindow;
    Singleton->Window->OnMousePositionChanged = OnMousePositionChangedWindow;
    Singleton->Window->OnMouseWheelChanged = OnMouseWheelChangedWindow;
    Singleton->Window->OnKeyChanged = OnKeyChangedWindow;

    Singleton->Width = Singleton->Window->Width;
    Singleton->Height = Singleton->Window->Height;

	// Initialize the graphics API
    gfx::context::initializeInfo ContextInitialize;
    ContextInitialize.Extensions = Singleton->Window->GetRequiredExtensions();
    ContextInitialize.ErrorCallback = ErrorCallback;
    ContextInitialize.InfoCallback = InfoCallback;
    ContextInitialize.Debug = true;
#if GFX_API == GFX_VK
    ContextInitialize.EnableRTX = true;
#endif
    Singleton->GfxContext = gfx::context::Initialize(ContextInitialize, *Singleton->Window);
    Singleton->Swapchain = Singleton->GfxContext->CreateSwapchain(Singleton->Width, Singleton->Height);
    Singleton->SwapchainPass = Singleton->GfxContext->GetDefaultRenderPass();


    Singleton->Imgui = gfx::imgui::Initialize(Singleton->GfxContext, Singleton->Window, Singleton->SwapchainPass);
    Singleton->GUI = std::make_shared<contextGUI>(Singleton.get());

    // Register pipelines that will be used in the app
    Singleton->Pipelines[PBRPipeline] = Singleton->GfxContext->CreatePipelineFromFile("resources/Hlgfx/Shaders/PBR/PBR.json");
    Singleton->Pipelines[ShadowsPipeline] = Singleton->GfxContext->CreatePipelineFromFile("resources/Hlgfx/Shaders/ShadowMaps/ShadowMaps.json");
    Singleton->Pipelines[GBufferPipeline] = Singleton->GfxContext->CreatePipelineFromFile("resources/Hlgfx/Shaders/Deferred/GBuffer.json");
    Singleton->Pipelines[CompositionPipeline] = Singleton->GfxContext->CreatePipelineFromFile("resources/Hlgfx/Shaders/Deferred/Composition.json");
#if GFX_API == GFX_VK
    Singleton->Pipelines[RTXReflectionsPipeline] = Singleton->GfxContext->CreatePipelineFromFile("resources/Hlgfx/Shaders/RTX/Reflections.json");
#endif

    struct rgba {uint8_t r, g, b, a;};
    u32 TexWidth = 64;
    u32 TexHeight = 64;
    std::vector<rgba> ColorData(TexWidth * TexHeight);
    
    rgba Color = {0,0,0,255};
    for (sz i = 0; i < TexWidth * TexHeight; i++)
        ColorData[i] = Color;
    
    gfx::imageData ImageData = {};
    ImageData.ChannelCount = 4;
    ImageData.Data = (u8*)ColorData.data();
    ImageData.DataSize = ColorData.size() * sizeof(rgba);
    ImageData.Width = TexWidth;
    ImageData.Height = TexHeight;
    ImageData.Format = gfx::format::R8G8B8A8_UNORM;
    ImageData.Type = gfx::type::UNSIGNED_BYTE;
    gfx::imageCreateInfo ImageCreateInfo = 
    {
        {0.0f,0.0f,0.0f,0.0f},
        gfx::samplerFilter::Linear,
        gfx::samplerFilter::Linear,
        gfx::samplerWrapMode::Repeat,
        gfx::samplerWrapMode::Repeat,
        gfx::samplerWrapMode::Repeat,
        true
    };    
    defaultTextures::BlackTexture = std::make_shared<texture>("Black", gfx::InvalidHandle);
    defaultTextures::BlackTexture->Handle = Singleton->GfxContext->CreateImage(ImageData, ImageCreateInfo);

    Color = {127,127,255,255};
    for (sz i = 0; i < TexWidth * TexHeight; i++)
        ColorData[i] = Color;
    
    defaultTextures::BlueTexture =std::make_shared<texture>("Blue", gfx::InvalidHandle);
    defaultTextures::BlueTexture->Handle = Singleton->GfxContext->CreateImage(ImageData, ImageCreateInfo);
    
    Color = {255,255,255,255};
    for (sz i = 0; i < TexWidth * TexHeight; i++)
        ColorData[i] = Color;

    defaultTextures::WhiteTexture = std::make_shared<texture>("White", gfx::InvalidHandle);
    defaultTextures::WhiteTexture->Handle = Singleton->GfxContext->CreateImage(ImageData, ImageCreateInfo);

    

#if 0 //TODO
    //Initialize default 3d objects WITHOUT materials. materials are created when these objects are instanciated.
    Singleton->Quad = GetQuadGeometry();
    Singleton->Quad->UUID = "DEFAULT_QUAD";

    Singleton->Cube = GetCubeGeometry();
    Singleton->Cube->UUID = "DEFAULT_CUBE";

    Singleton->Sphere = GetSphereGeometry();
    Singleton->Sphere->UUID = "DEFAULT_SPHERE";

    Singleton->Cone = GetConeGeometry();
    Singleton->Cone->UUID = "DEFAULT_CONE";

    Singleton->Capsule = GetCapsuleGeometry();
    Singleton->Capsule->UUID = "DEFAULT_CAPSULE";
    //We need a way of knowing that the main render pass depends on the shadows render pass
    //so that it can transition all the resources it needs to shader read]
#endif

    //Only one for directinoal shadows
    Singleton->ShadowMaps = Singleton->GfxContext->CreateImageArray(ShadowMapSize, ShadowMapSize, MaxLights, ShadowMapFormat, gfx::imageUsage::SHADER_READ);
    
    Singleton->ShadowsRenderer = std::make_shared<hlgfx::shadowsRenderer>();
    
    if(Singleton->RenderType == rendererType::forward)
        Singleton->MainRenderer = std::make_shared<hlgfx::mainRenderer>();
    else if(Singleton->RenderType == rendererType::deferred)
        Singleton->MainRenderer = std::make_shared<hlgfx::deferredRenderer>();

#if 0 //TODO
    Singleton->NoMaterial = std::make_shared<pbrMaterial>("NO_MATERIAL");
    Singleton->NoMaterial->UUID = "NO_MATERIAL";
#endif

    return Singleton;
}

gfx::pipelineCreation context::GetPipelineCreation(materialFlags::bits Flags, gfx::renderPassHandle RenderPassHandle)
{
    // This is odd. We should get all of that from the json file instead..

    gfx::pipelineCreation PipelineCreation = {};

    std::string ShaderParentPath;
    std::string ShaderFileName;
    if(Flags & materialFlags::PBR)
    {
        PipelineCreation.Name = gfx::AllocateCString("PBR");
        ShaderParentPath = "resources/Hlgfx/Shaders/PBR/";
        if(GFX_API == GFX_VK || GFX_API == GFX_GL)
        {
            ShaderFileName = "PBR.glsl";
        }
        else
        {
            ShaderFileName = "PBR.hlsl";
        }
    } else if(Flags & materialFlags::GBuffer)
    {
        PipelineCreation.Name = gfx::AllocateCString("GBuffer");
        ShaderParentPath = "resources/Hlgfx/Shaders/Deferred/";
        if(GFX_API == GFX_VK || GFX_API == GFX_GL)
        {
            ShaderFileName = "GBuffer.glsl";
        }
        else
        {
            ShaderFileName = "GBuffer.hlsl";
        }        
    }


    //
    std::string ApiDefinition;
    ApiDefinition += "#define VK " + std::to_string(GFX_VK) + "\n";
    ApiDefinition += "#define GL " + std::to_string(GFX_GL) + "\n";
    ApiDefinition += "#define D3D11 " + std::to_string(GFX_D3D11) + "\n";
    ApiDefinition += "#define D3D12 " + std::to_string(GFX_D3D12) + "\n";
    if(GFX_API == GFX_VK)
    {
        ApiDefinition += "#define GRAPHICS_API VK\n";
    }
    if(GFX_API == GFX_GL)
    {
        ApiDefinition += "#define GRAPHICS_API GL\n";
    }
    if(GFX_API == GFX_D3D12)
    {
        ApiDefinition += "#define GRAPHICS_API D3D12\n";
    }
    if(GFX_API == GFX_D3D11)
    {
        ApiDefinition += "#define GRAPHICS_API D3D11\n";
    }

    std::string VertexCode;
    gfx::ShaderConcatenate(ShaderFileName, VertexCode, ShaderParentPath);
    std::string VertexCustomDefines = "#define VERTEX\n";
    VertexCustomDefines += ApiDefinition;
    VertexCode = std::regex_replace(VertexCode, std::regex("// CUSTOM_DEFINES"), VertexCustomDefines);
    const char *VertexCodeCStr = gfx::AllocateCString(VertexCode.c_str());
    const char *VertexFileNameCStr = gfx::AllocateCString(ShaderParentPath + "/" + ShaderFileName);
    PipelineCreation.Shaders.AddStage(VertexCodeCStr, VertexFileNameCStr, (u32)strlen(VertexCodeCStr), gfx::shaderStageFlags::bits::Vertex);

    std::string FragmentCode;
    gfx::ShaderConcatenate(ShaderFileName, FragmentCode, ShaderParentPath);
    std::string FragmentCustomDefines = "#define FRAGMENT\n";
    FragmentCustomDefines += ApiDefinition;
    FragmentCode = std::regex_replace(FragmentCode, std::regex("// CUSTOM_DEFINES"), FragmentCustomDefines);
    const char *FragmentCodeCStr = gfx::AllocateCString(FragmentCode.c_str());
    const char *FragmentFileNameCStr = gfx::AllocateCString(ShaderParentPath + "/" + ShaderFileName);
    PipelineCreation.Shaders.AddStage(FragmentCodeCStr, FragmentFileNameCStr, (u32)strlen(FragmentCodeCStr), gfx::shaderStageFlags::bits::Fragment);

    
    PipelineCreation.VertexInput.NumVertexStreams=0;
    gfx::vertexStream VertexStream{};
    VertexStream.Binding = 0;
    VertexStream.Stride = sizeof(vertex);
    VertexStream.InputRate = gfx::vertexInputRate::PerVertex;
    PipelineCreation.VertexInput.AddVertexStream(VertexStream);        

    PipelineCreation.VertexInput.NumVertexAttributes=0;
    gfx::vertexAttribute VertexAttribute0{};
    VertexAttribute0.Location = 0;
    VertexAttribute0.Binding = 0;
    VertexAttribute0.Offset = 0;
    VertexAttribute0.Format = gfx::vertexComponentFormat::Float4;
    VertexAttribute0.SemanticIndex = 0;
    PipelineCreation.VertexInput.AddVertexAttribute(VertexAttribute0);
    gfx::vertexAttribute VertexAttribute1{};
    VertexAttribute1.Location = 1;
    VertexAttribute1.Binding = 0;
    VertexAttribute1.Offset = sizeof(v4f);
    VertexAttribute1.Format = gfx::vertexComponentFormat::Float4;
    VertexAttribute1.SemanticIndex = 1;
    PipelineCreation.VertexInput.AddVertexAttribute(VertexAttribute1);
    gfx::vertexAttribute VertexAttribute2{};
    VertexAttribute1.Location = 2;
    VertexAttribute1.Binding = 0;
    VertexAttribute1.Offset = 2 * sizeof(v4f);
    VertexAttribute1.Format = gfx::vertexComponentFormat::Float4;
    VertexAttribute1.SemanticIndex = 2;
    PipelineCreation.VertexInput.AddVertexAttribute(VertexAttribute1);

    u8 T = Flags & materialFlags::DepthTestEnabled;

    PipelineCreation.DepthStencil.DepthEnable = (u8)((b8)(Flags & materialFlags::DepthTestEnabled));
    PipelineCreation.DepthStencil.DepthWriteEnable = (u8)((b8)(Flags & materialFlags::DepthWriteEnabled));
    PipelineCreation.DepthStencil.DepthComparison = gfx::compareOperation::LessOrEqual;
    
    // TODO: If GBuffer, add blend states
    if(Flags & materialFlags::GBuffer)
    {
        for(int i=0; i<4; i++)
        {
            gfx::blendState &BlendState = PipelineCreation.BlendState.AddBlendState();
            BlendState.BlendEnabled = false;
            // BlendState.BlendEnabled = Flags & materialFlags::BlendEnabled;
            // BlendState.SeparateBlend=false;
            // if(BlendState.BlendEnabled)
            //     BlendState.SetColor(gfx::blendFactor::SrcAlpha, gfx::blendFactor::OneMinusSrcAlpha, gfx::blendOperation::Add);
        }
    }
    else
    {
        gfx::blendState &BlendState = PipelineCreation.BlendState.AddBlendState();
        BlendState.BlendEnabled = Flags & materialFlags::BlendEnabled;
        BlendState.SeparateBlend=false;
        if(BlendState.BlendEnabled)
            BlendState.SetColor(gfx::blendFactor::SrcAlpha, gfx::blendFactor::OneMinusSrcAlpha, gfx::blendOperation::Add);
    }

    PipelineCreation.Rasterization.CullMode = (Flags & materialFlags::CullModeOn) ? gfx::cullMode::Back : gfx::cullMode::None;

    
    PipelineCreation.RenderPassHandle = (RenderPassHandle == gfx::InvalidHandle) ? gfx::context::Get()->SwapchainRenderPass : RenderPassHandle;

    return PipelineCreation;    
}

gfx::pipelineHandle context::GetPipeline(materialFlags::bits Flags)
{
    if(this->AllPipelines.find(Flags) != this->AllPipelines.end())
    {
        return this->AllPipelines[Flags];    
    }
    return gfx::InvalidHandle;
}

gfx::pipelineHandle context::CreateOrGetPipeline(materialFlags::bits Flags)
{
    if(this->AllPipelines.find(Flags) == this->AllPipelines.end())
    {
        gfx::renderPassHandle RenderPass = GfxContext->GetFramebuffer(this->MainRenderer->RenderTarget)->RenderPass;
        gfx::pipelineCreation Creation = GetPipelineCreation(Flags, RenderPass);
        this->AllPipelines[Flags] = gfx::context::Get()->CreatePipeline(Creation);
    }

    return this->AllPipelines[Flags];
}

void context::StartFrame()
{
    this->MouseClicked=false;
    this->MouseReleased=false;
    this->MouseMoved=false;
    this->MouseWheelChanged=false;

    Window->PollEvents();
    
    this->Scene->OnEarlyUpdate();
    
    GfxContext->StartFrame();	
    
    std::shared_ptr<gfx::commandBuffer> CommandBuffer = GfxContext->GetCurrentFrameCommandBuffer();
    CommandBuffer->Begin();

}

std::string context::GetUUID()
{
    UUIDv4::UUID UUID = UUIDGenerator.getUUID();
    std::string Result = UUID.bytes();
    return Result;
}

void context::AddTextureToProject(std::shared_ptr<texture> Texture)
{
    //Check unicity of name
    u32 Count = 0;
    std::string BaseName = Texture->Name;
    while(true)
    {
        b8 OK = true;
        for (auto &ProjectTexture : this->Project.Textures)
        {
            //If another object has the same name, we append the count, and set OK to false so that we iterate again for this new name
            if(ProjectTexture->Name == Texture->Name)
            {
                Texture->Name = BaseName + "_" + std::to_string(Count);
                OK = false;
                Count++;
            }
        }
        if(OK) break;
    }    

    this->Project.Textures[Texture->ID] = Texture;
}

void context::RemoveTextureFromProject(std::shared_ptr<texture> Texture)
{
#if 0 //todo
    //Remove from project
    this->Project.Textures.erase(Texture->ID);

    //remove from all materials
    for (auto &Material : this->Project.Materials)
    {
        std::shared_ptr<pbrMaterial> PBRMaterial = std::dynamic_pointer_cast<pbrMaterial>(Material.second);
        if(PBRMaterial)
        {
            if(PBRMaterial->BaseColorTexture.get() == Texture.get())
            {
                PBRMaterial->SetBaseColorTexture(defaultTextures::BlackTexture);
            }
            if(PBRMaterial->MetallicRoughnessTexture.get() == Texture.get())
            {
                PBRMaterial->SetMetallicRoughnessTexture(defaultTextures::BlackTexture);
            }
            if(PBRMaterial->NormalTexture.get() == Texture.get())
            {
                PBRMaterial->SetNormalTexture(defaultTextures::BlackTexture);
            }
            if(PBRMaterial->OcclusionTexture.get() == Texture.get())
            {
                PBRMaterial->SetOcclusionTexture(defaultTextures::BlackTexture);
            }
            if(PBRMaterial->EmissiveTexture.get() == Texture.get())
            {
                PBRMaterial->SetEmissiveTexture(defaultTextures::BlackTexture);
            }
        }
    }
#endif
}

void context::AddMaterialToProject(std::shared_ptr<material> Material)
{
#if 0 //todo
    if(this->Project.Materials.find(Material->ID) != this->Project.Materials.end()) return;
    //Check unicity of name
    u32 Count = 0;
    std::string BaseName = Material->Name;
    while(true)
    {
        b8 OK = true;
        for (auto &ProjectMaterial : this->Project.Materials)
        {
            //If another object has the same name, we append the count, and set OK to false so that we iterate again for this new name
            if(ProjectMaterial.second->Name == Material->Name)
            {
                Material->Name = BaseName + "_" + std::to_string(Count);
                OK = false;
                Count++;
            }
        }

        if(OK) break;
    }

    this->Project.Materials[Material->ID] = Material;
#endif
}

void context::RemoveMaterialFromProject(std::shared_ptr<material> Material)
{
#if 0 //todo
    //remove from materiails
    this->Project.Materials.erase(Material->ID);
    //remove from all objects
    for(auto &Object : this->Project.Objects)
    {
        std::queue<std::shared_ptr<object3D>> ToCheck;
        ToCheck.push(Object.second);
        while(!ToCheck.empty())
        {
            std::shared_ptr<object3D> Obj = ToCheck.back();
            ToCheck.pop();
            std::shared_ptr<mesh> Mesh = std::dynamic_pointer_cast<mesh>(Obj);
            if(Mesh)
            {
                if(Mesh->Material.get() == Material.get())
                {
                    Mesh->Material = this->NoMaterial;
                }
            }

            for (sz i = 0; i < Obj->Children.size(); i++)
            {
                ToCheck.push(Obj->Children[i]);
            }
        }
    }

    //remove from all scenes
    for(auto &Scene : this->Project.Scenes)
    {
        std::queue<std::shared_ptr<object3D>> ToCheck;
        ToCheck.push(Scene.second);
        while(!ToCheck.empty())
        {
            std::shared_ptr<object3D> Obj = ToCheck.back();
            ToCheck.pop();
            std::shared_ptr<mesh> Mesh = std::dynamic_pointer_cast<mesh>(Obj);
            if(Mesh)
            {
                if(Mesh->Material.get() == Material.get())
                {
                    Mesh->Material = this->NoMaterial;
                }
            }

            for (sz i = 0; i < Obj->Children.size(); i++)
            {
                ToCheck.push(Obj->Children[i]);
            }
        }
    }    
#endif
}

void context::AddGeometryToProject(std::shared_ptr<indexedGeometryBuffers> Geometry)
{
#if 0 //todo
    if(this->Project.Geometries.find(Geometry->ID) != this->Project.Geometries.end()) return;

    //Check unicity of name
    u32 Count = 0;
    std::string BaseName = Geometry->Name;
    while(true)
    {
        b8 OK = true;
        for (auto &ProjectGeometry : this->Project.Geometries)
        {
            //If another object has the same name, we append the count, and set OK to false so that we iterate again for this new name
            if(ProjectGeometry.second->Name == Geometry->Name)
            {
                Geometry->Name = BaseName + "_" + std::to_string(Count);
                OK = false;
                Count++;
            }
        }

        if(OK) break;
    }

    this->Project.Geometries[Geometry->ID] = Geometry;
#endif
}

void context::AddMeshToProject(std::shared_ptr<mesh> Mesh)
{
#if 0 //TODO
    AddMaterialToProject(Mesh->Material);
    AddGeometryToProject(Mesh->GeometryBuffers);
    for(auto &Texture : Mesh->Material->AllTextures)
    {
        AddTextureToProject(Texture.second);
    }
#endif
}

void context::AddSceneToProject(std::shared_ptr<scene> Scene)
{
    //Check unicity of name
    u32 Count = 0;
    std::string BaseName = Scene->Name;
    while(true)
    {
        b8 OK = true;
        for (auto &ProjectScene : this->Project.Scenes)
        {
            //If another object has the same name, we append the count, and set OK to false so that we iterate again for this new name
            if(ProjectScene->Name == Scene->Name)
            {
                Scene->Name = BaseName + "_" + std::to_string(Count);
                OK = false;
                Count++;
            }
        }

        if(OK) break;
    }
    if (Project.Scenes.size() < Scene->ID + 1)
    {
        Project.Scenes.resize(Scene->ID + 1);
    }
    this->Project.Scenes[Scene->ID] = Scene;
    this->Scene = Scene;
}

void context::RemoveObjectFromProject(std::shared_ptr<object3D> Object)
{
#if 0 //todo
    this->Project.Objects.erase(Object->ID);
#endif
}
  
void context::AddObjectToProject(std::shared_ptr<object3D> Object, u32 Level)
{
    if (Level == 0)
    {
        //Check unicity of name
        u32 Count = 0;
        std::string BaseName = Object->Name;
        while(true)
        {
            b8 OK = true;
            for (auto &ProjectObject : this->Project.Objects)
            {
                //If another object has the same name, we append the count, and set OK to false so that we iterate again for this new name
                if(ProjectObject->Name == Object->Name)
                {
                    Object->Name = BaseName + "_" + std::to_string(Count);
                    OK = false;
                    Count++;
                }
            }

            if(OK) break;
        }
        this->Project.Objects[Object->ID] = (Object);
    }
    
    
    std::shared_ptr<mesh> Mesh = std::dynamic_pointer_cast<mesh>(Object);
    if(Mesh)
    {
        AddMeshToProject(Mesh);
    } 
    for (sz i = 0; i < Object->Children.size(); i++)
    {
        AddObjectToProject(Object->Children[i], 1);
    }
}

void context::NewProject()
{
    this->Project.Geometries.clear();
    this->Project.Materials.clear();
    this->Project.Objects.clear();
    this->Project.Textures.clear();
    this->Project.Scenes.clear();
    this->Scene = nullptr;

    this->Scene = std::make_shared<scene>("New Scene");
}

void context::Render(std::shared_ptr<camera> Camera)
{
    Frame++;

    this->CurrentCamera = Camera;
    Camera->Controls->OnUpdate();
    
    std::shared_ptr<gfx::commandBuffer> CommandBuffer = GfxContext->GetCurrentFrameCommandBuffer();    
    
    //Render shadow maps
    for(u32 i=0; i<Scene->SceneBufferData.LightCount; i++)
    {

        if(s32(Scene->Lights[i]->Data.SizeAndType.w) == s32(light::lightType::Directional)) 
        {        
            this->ShadowsRenderer->OverrideMaterial = Scene->Lights[i]->Material;
            this->ShadowsRenderer->RenderTarget = Scene->Lights[i]->ShadowsFramebuffer;

            Scene->Lights[i]->CalculateMatrices(Camera);
            Scene->UpdateLight(i);

            //Shadow pass
            ShadowsRenderer->Render(Scene, Scene->Lights[i]->ShadowCam);

            //Copy the framebuffer to the texture layer
            CommandBuffer->CopyFramebufferToImage(
                gfx::framebufferInfo {gfx::context::Get()->GetFramebuffer(Scene->Lights[i]->ShadowsFramebuffer), gfx::imageUsage::UNKNOWN, true, 0 },
                gfx::imageInfo {gfx::context::Get()->GetImage(this->ShadowMaps), gfx::imageUsage::UNKNOWN, 0, i, 1}
            );
        }
    }
    
    // Render with main renderer
    
    MainRenderer->Render(Scene, Camera);

    // Render with deferred renderer

    // 
}

void context::Update()
{
    this->Scene->OnUpdate();
}

void context::EndFrame()
{
    GfxContext->EndFrame();
    GfxContext->Present();   
     
}

void context::SetRenderFlags(materialFlags::bits &Flags)
{
    if(this->RenderType == rendererType::deferred)
    {
        if(Flags & materialFlags::bits::PBR)
            Flags = (materialFlags::bits)((u32)Flags & ~(u32)materialFlags::bits::PBR);
        if(!(Flags & materialFlags::bits::GBuffer))
            Flags = (materialFlags::bits)((u32)Flags |  (u32)materialFlags::bits::GBuffer);
    }
    else
    {
        if(Flags & materialFlags::bits::GBuffer)
            Flags = (materialFlags::bits)((u32)Flags & ~(u32)materialFlags::bits::GBuffer);
        if(!(Flags & materialFlags::bits::PBR))
            Flags = (materialFlags::bits)((u32)Flags |  (u32)materialFlags::bits::PBR);
    }
}


void context::OnResize(u32 Width, u32 Height)
{
    GfxContext->OnResize(Width, Height);
}

void context::OnMouseClicked(app::mouseButton Button, bool Clicked)
{
    Imgui->OnClick(Button, Clicked);
    
    if(Clicked) 
    {
        this->MouseClicked=true;
        if(Button == app::mouseButton::LEFT) this->LeftButtonPressed=true;
        if(Button == app::mouseButton::RIGHT) this->RightButtonPressed=true;
    }
    if(!Clicked) 
    {
        this->MouseReleased=true;
        if(Button == app::mouseButton::LEFT) this->LeftButtonPressed=false;
        if(Button == app::mouseButton::RIGHT) this->RightButtonPressed=false;
    }
    this->ButtonClicked = Button;
}

void context::OnMousePositionChanged(f64 NewPosX, f64 NewPosY)
{
    this->MouseMoved=true;
    this->MouseDelta.x = NewPosX - this->MousePosition.x;
    this->MouseDelta.y = NewPosY - this->MousePosition.y;
    this->MousePosition.x = NewPosX;
    this->MousePosition.y = NewPosY;
}

void context::OnMouseWheelChanged(f64 OffsetX, f64 OffsetY)
{
    this->MouseWheelChanged=true;
    this->MouseWheelX = OffsetX;
    this->MouseWheelY = OffsetY;
}

void context::OnKeyChanged(app::keyCode KeyCode, b8 KeyDown)
{
    Imgui->OnKeyChanged(KeyCode, KeyDown);
}

void context::Cleanup()
{
    GfxContext->WaitIdle();
    
    this->MainRenderer = nullptr;

    //Clean project
    this->Project.Scenes.clear();
    this->Project.Objects.clear();
    this->Project.Geometries.clear();
    this->Project.Materials.clear();
    this->Project.Textures.clear();
    for (size_t i = 0; i < this->Scene->SceneBufferData.LightCount; i++)
    {
        this->Scene->Lights[i] = nullptr;
    }
    
    this->Scene = nullptr;

    CurrentCamera = nullptr;

    GfxContext->QueueDestroyImage(ShadowMaps);

#if 0 //TODO
    Quad->Destroy();
    Cube->Destroy();
    Sphere->Destroy();
    Cone->Destroy();
    Capsule->Destroy();
#endif

    this->Quad = nullptr;
    
    NoMaterial = nullptr;

    //TODO: Is that necessary ?
    GfxContext->DestroyImage(defaultTextures::BlackTexture->Handle);
    GfxContext->DestroyImage(defaultTextures::WhiteTexture->Handle);
    GfxContext->DestroyImage(defaultTextures::BlueTexture->Handle);

    for(auto &Pipeline : this->AllPipelines)
    {
        GfxContext->DestroyPipeline(Pipeline.second);
    }
    for(auto &Pipeline : this->Pipelines)
    {
        GfxContext->DestroyPipeline(Pipeline.second);
    }
    GfxContext->ProcessDeletionQueue();

    Imgui->Cleanup();
    GfxContext->DestroySwapchain();
    GfxContext->Cleanup();

    gfx::memory *Memory = gfx::memory::Get();
    Memory->Destroy();
    delete Memory;    
}

void context::LoadProjectFromFile(const char *FileName)
{
    this->Project.Geometries.clear();
    this->Project.Scenes.clear();
    this->Project.Objects.clear();
    this->Project.Textures.clear();
    this->Project.Scenes.clear();

    std::string FolderName = std::filesystem::path(FileName).parent_path().string() + "/Assets";

    std::vector<std::string> TextureFiles;
    std::vector<std::string> MaterialFiles;
    std::vector<std::string> GeometryFiles;
    std::vector<std::string> ObjectFiles;
    std::vector<std::string> SceneFiles;
    
    std::ifstream ProjectFileStream;
    ProjectFileStream.open(FileName, std::ios_base::binary);
    assert(ProjectFileStream.is_open());

    namespace fs = std::filesystem;
    u32 FilesCount;
    ProjectFileStream.read((char*)&FilesCount, sizeof(u32));
    for (u32 i = 0; i < FilesCount; i++)
    {
        sz StringLength;
        ProjectFileStream.read((char*)&StringLength, sizeof(sz));
        std::string File; File.resize(StringLength);
        ProjectFileStream.read(File.data(), File.size());

        fs::directory_entry Entry = fs::directory_entry(FolderName +"/" + File);
        std::string FileName = Entry.path().filename().string();
        std::string Extension = Entry.path().extension().string();
        
        if (fs::is_regular_file(Entry)) {
            if(Extension == ".mat")
            {
                MaterialFiles.push_back(Entry.path().string());
            }
            else if(Extension == ".tex")
            {
                TextureFiles.push_back(Entry.path().string());
            }
            else if(Extension == ".geom")
            {
                GeometryFiles.push_back(Entry.path().string());
            }
            else if(Extension == ".obj")
            {
                ObjectFiles.push_back(Entry.path().string());
            }
            else if(Extension == ".scene")
            {
                SceneFiles.push_back(Entry.path().string());
            }
        }
    }
    


    for (sz i = 0; i < TextureFiles.size(); i++)
    {
        std::shared_ptr<texture> Texture = texture::Deserialize(TextureFiles[i]);
        if(Project.Textures.size() < Texture->ID + 1)
        {
            Project.Textures.resize(Texture->ID + 1);
        }             
        Project.Textures[Texture->ID] = Texture;   
    }

    for (sz i = 0; i < MaterialFiles.size(); i++)
    {
        std::shared_ptr<material> Material = material::Deserialize(MaterialFiles[i]);
        if(Project.Materials.size() < Material->ID + 1)
        {
            Project.Materials.resize(Material->ID + 1);
        }            
        Project.Materials[Material->ID] = Material;
    }
    
    for (sz i = 0; i < GeometryFiles.size(); i++)
    {
        std::shared_ptr<indexedGeometryBuffers> Geometry = indexedGeometryBuffers::Deserialize(GeometryFiles[i]);
        if(Project.Geometries.size() < Geometry->ID + 1)
        {
            Project.Geometries.resize(Geometry->ID + 1);
        }            
        Project.Geometries[Geometry->ID] = Geometry;
    }
    
    for (sz i = 0; i < ObjectFiles.size(); i++)
    {
        std::shared_ptr<object3D> Object = object3D::Deserialize(ObjectFiles[i]);
        if(Project.Objects.size() < Object->ID + 1)
        {
            Project.Objects.resize(Object->ID + 1);
        }            
        Project.Objects[Object->ID] = Object;   
    }
    
    for (sz i = 0; i < SceneFiles.size(); i++)
    {
        std::shared_ptr<scene> Scene = std::static_pointer_cast<scene>(object3D::Deserialize(SceneFiles[i]));
        if(Project.Scenes.size() < Scene->ID + 1)
        {
            Project.Scenes.resize(Scene->ID + 1);
        }            
        Project.Scenes[Scene->ID] = Scene;  
        if(i==0) 
        {
            this->Scene = Scene;
            if(this->GfxContext->RTXEnabled)
            {
                Scene->BuildTLAS();
                this->MainRenderer->SceneUpdate();
            }
        }
    }


    // We need all those materials to be using the deferred pipeline.
    // 

    // if(std::shared_ptr<deferredRenderer> DeferredRenderer =  std::dynamic_pointer_cast<deferredRenderer>(Singleton->MainRenderer))
    // {
    //     gfx::renderPassHandle RenderPass = gfx::context::Get()->GetFramebuffer(DeferredRenderer->RenderTarget)->RenderPass;
        
    //     // Convert all pipelines in the scene to GBuffer pipeline
    //     std::unordered_map<materialFlags::bits, gfx::pipelineHandle> NewPipelines;
    //     for(auto &Pipeline : AllPipelines)
    //     {
    //         materialFlags::bits Flag = Pipeline.first;

    //         Flag = (materialFlags::bits)((u32)Flag & ~(u32)materialFlags::bits::PBR);
    //         Flag = (materialFlags::bits)((u32)Flag |  (u32)materialFlags::bits::GBuffer);
    //         gfx::pipelineCreation PipelineCreation = context::Get()->GetPipelineCreation(Flag, RenderPass);
    //         gfx::context::Get()->RecreatePipeline(PipelineCreation, Pipeline.second);

    //         NewPipelines[Flag] = Pipeline.second;
    //     }
    //     this->AllPipelines= NewPipelines;

    //     // Change the materials so they bind with the new materials
    //     for(auto &Material : Singleton->Project.Materials)
    //     {
    //         Material.second->Flags = (materialFlags::bits)((u32)Material.second->Flags & ~(u32)materialFlags::bits::PBR);
    //         Material.second->Flags = (materialFlags::bits)((u32)Material.second->Flags |  (u32)materialFlags::bits::GBuffer);
    //         gfx::context::Get()->BindUniformsToPipeline(Material.second->Uniforms, Material.second->PipelineHandle, MaterialDescriptorSetBinding, true);
    //         Material.second->Uniforms->Update();                     
    //     }

    //     for(auto &PipelineMeshes : Singleton->Scene->Meshes)
    //     {
    //         for(auto &Mesh : PipelineMeshes.second)
    //         {
    //             gfx::context::Get()->BindUniformsToPipeline(Mesh->Uniforms, Mesh->Material->PipelineHandle, ModelDescriptorSetBinding, true);
    //             Mesh->Uniforms->Update();
    //         }
    //     }
    // }    
}

void context::SaveProjectToFile(const char *FileName)
{
    std::string FolderName = std::filesystem::path(FileName).parent_path().string() + "/Assets";

    if (!std::filesystem::exists(FolderName)) 
    {
        if (!std::filesystem::create_directory(FolderName))
        {
            std::cerr << "Error creating directory." << std::endl;
        }
    }

    std::vector<std::string> AllFiles;
    //Save all the materials
    for (auto &Material : Project.Materials)
    {
        std::string FileName = Material->Name + ".mat";
        Material->Serialize(FolderName + "/" + FileName);
        AllFiles.push_back(FileName);
    }
    
    //Save all the textures
    for (auto &Texture : Project.Textures)
    {
        std::string FileName = Texture->Name + ".tex";
        Texture->Serialize(FolderName + "/" + FileName);
        AllFiles.push_back(FileName);
    }

    //Save all the geometries
    for (auto &Geometry : Project.Geometries)
    {
        std::string FileName = Geometry->Name + ".geom";
        Geometry->Serialize(FolderName + "/" + FileName);
        AllFiles.push_back(FileName);
    }

    //Save all the objects
    for (auto &Object : Project.Objects)
    {
        std::string FileName = Object->Name + ".obj";
        Object->Serialize(FolderName + "/" + FileName);
        AllFiles.push_back(FileName);
    }

    //Save all the scenes
    for (auto &Scene : Project.Scenes)
    {
        std::string FileName = Scene->Name + ".scene";
        Scene->Serialize(FolderName + "/" +FileName) ;
        AllFiles.push_back(FileName);
    }

    std::ofstream ProjectFileStream;
    ProjectFileStream.open(FileName, std::ios_base::trunc | std::ios_base::binary);
    assert(ProjectFileStream.is_open());

    u32 FilesCount = AllFiles.size();
    ProjectFileStream.write((char*)&FilesCount, sizeof(u32));
    for (sz i = 0; i < AllFiles.size(); i++)
    {
        sz Size = AllFiles[i].size();
        ProjectFileStream.write((char*)&Size, sizeof(sz));
        ProjectFileStream.write(AllFiles[i].data(), AllFiles[i].size());
    }
}

context::~context()
{
    Cleanup();
}

}