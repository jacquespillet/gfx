#include "Include/Context.h"
#include "Include/Scene.h"
#include "Include/CameraController.h"
#include "Include/Material.h"
#include "Include/Util.h"
#include "Include/GUI.h"
#include "Loaders/GLTF.h"
#include "Gfx/Include/CommandBuffer.h"
#include "Gfx/Common/Util.h"
#include <iostream>
#include <filesystem>
#include <queue>

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

    Singleton->Width = Singleton->Window->Width;
    Singleton->Height = Singleton->Window->Height;

	// Initialize the graphics API
    gfx::context::initializeInfo ContextInitialize;
    ContextInitialize.Extensions = Singleton->Window->GetRequiredExtensions();
    ContextInitialize.ErrorCallback = ErrorCallback;
    ContextInitialize.InfoCallback = InfoCallback;
    ContextInitialize.Debug = true;
    Singleton->GfxContext = gfx::context::Initialize(ContextInitialize, *Singleton->Window);
    Singleton->Swapchain = Singleton->GfxContext->CreateSwapchain(Singleton->Width, Singleton->Height);
    Singleton->SwapchainPass = Singleton->GfxContext->GetDefaultRenderPass();


    Singleton->Imgui = gfx::imgui::Initialize(Singleton->GfxContext, Singleton->Window, Singleton->SwapchainPass);
    Singleton->GUI = std::make_shared<contextGUI>(Singleton.get());
    Singleton->Pipelines[UnlitPipeline] = gfx::context::Get()->CreatePipelineFromFile("resources/Hlgfx/Shaders/Unlit/Unlit.json");

    
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

    defaultTextures::BlackTexture->Handle = Singleton->GfxContext->CreateImage(ImageData, ImageCreateInfo);

    Color = {0,0,255,255};
    for (sz i = 0; i < TexWidth * TexHeight; i++)
        ColorData[i] = Color;
    defaultTextures::BlueTexture->Handle = Singleton->GfxContext->CreateImage(ImageData, ImageCreateInfo);
    
    Color = {255,255,255,255};
    for (sz i = 0; i < TexWidth * TexHeight; i++)
        ColorData[i] = Color;
    defaultTextures::WhiteTexture->Handle = Singleton->GfxContext->CreateImage(ImageData, ImageCreateInfo);

    Singleton->NoMaterial = std::make_shared<unlitMaterial>("NO_MATERIAL");
    Singleton->NoMaterial->UUID = "NO_MATERIAL";
    return Singleton;
}

gfx::pipelineCreation context::GetPipelineCreation(materialFlags::bits Flags)
{

    gfx::pipelineCreation PipelineCreation = {};

    std::string ShaderParentPath;
    std::string ShaderFileName;
    if(Flags & materialFlags::Unlit)
    {
        PipelineCreation.Name = gfx::AllocateCString("Unlit");
        ShaderParentPath = "resources/Hlgfx/Shaders/Unlit/";
        if(GFX_API == GFX_VK || GFX_API == GFX_GL)
        {
            ShaderFileName = "Unlit.glsl";
        }
        else
        {
            ShaderFileName = "Unlit.hlsl";
        }
    }


    //
    std::string ApiDefinition;
    ApiDefinition += "#define VK " + std::to_string(GFX_VK) + "\n";
    ApiDefinition += "#define GL " + std::to_string(GFX_GL) + "\n";
    ApiDefinition += "#define D3D11 " + std::to_string(GFX_D3D11) + "\n";
    ApiDefinition += "#define GFX_D3D12 " + std::to_string(GFX_D3D12) + "\n";
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
    VertexCode = std::regex_replace(VertexCode, std::regex("CUSTOM_DEFINES"), VertexCustomDefines);
    const char *VertexCodeCStr = gfx::AllocateCString(VertexCode.c_str());
    const char *VertexFileNameCStr = gfx::AllocateCString(ShaderParentPath + "/" + ShaderFileName);
    PipelineCreation.Shaders.AddStage(VertexCodeCStr, VertexFileNameCStr, (u32)strlen(VertexCodeCStr), gfx::shaderStageFlags::bits::Vertex);

    std::string FragmentCode;
    gfx::ShaderConcatenate(ShaderFileName, FragmentCode, ShaderParentPath);
    std::string FragmentCustomDefines = "#define FRAGMENT\n";
    FragmentCustomDefines += ApiDefinition;
    FragmentCode = std::regex_replace(FragmentCode, std::regex("CUSTOM_DEFINES"), FragmentCustomDefines);
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

    u8 T = Flags & materialFlags::DepthTestEnabled;

    PipelineCreation.DepthStencil.DepthEnable = (u8)((b8)(Flags & materialFlags::DepthTestEnabled));
    PipelineCreation.DepthStencil.DepthWriteEnable = (u8)((b8)(Flags & materialFlags::DepthWriteEnabled));
    PipelineCreation.DepthStencil.DepthComparison = gfx::compareOperation::LessOrEqual;
    

    gfx::blendState &BlendState = PipelineCreation.BlendState.AddBlendState();
    BlendState.BlendEnabled = Flags & materialFlags::BlendEnabled;
    BlendState.SeparateBlend=false;
    if(BlendState.BlendEnabled)
        BlendState.SetColor(gfx::blendFactor::SrcAlpha, gfx::blendFactor::OneMinusSrcAlpha, gfx::blendOperation::Add);

    PipelineCreation.Rasterization.CullMode = (Flags & materialFlags::CullModeOn) ? gfx::cullMode::Back : gfx::cullMode::None;

    PipelineCreation.RenderPassHandle = gfx::context::Get()->SwapchainRenderPass;

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
        gfx::pipelineCreation Creation = GetPipelineCreation(Flags);
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

    CommandBuffer->BeginPass(GfxContext->GetSwapchainFramebuffer(), {0.5f, 0.0f, 0.8f, 1.0f}, {1.0f, 0});
    CommandBuffer->SetViewport(0.0f, 0.0f, (float)Width, (float)Height);
    CommandBuffer->SetScissor(0, 0, Width, Height);

    Imgui->StartFrame();
    this->GUI->StartFrame();


    this->GUI->DrawGUI();
}

std::string context::GetUUID()
{
    UUIDv4::UUID UUID = UUIDGenerator.getUUID();
    std::string Result = UUID.bytes();
    return Result;
}

void context::AddTextureToProject(std::shared_ptr<texture> Texture)
{
    if(this->Project.Textures.find(Texture->UUID) != this->Project.Textures.end()) return;

    //Check unicity of name
    u32 Count = 0;
    std::string BaseName = Texture->Name;
    while(true)
    {
        b8 OK = true;
        for (auto &ProjectTexture : this->Project.Textures)
        {
            //If another object has the same name, we append the count, and set OK to false so that we iterate again for this new name
            if(ProjectTexture.second->Name == Texture->Name)
            {
                Texture->Name = BaseName + "_" + std::to_string(Count);
                OK = false;
                Count++;
            }
        }
        if(OK) break;
    }    

    this->Project.Textures[Texture->UUID] = Texture;
}

void context::RemoveTextureFromProject(std::shared_ptr<texture> Texture)
{
    //Remove from project
    this->Project.Textures.erase(Texture->UUID);

    //remove from all materials
    for (auto &Material : this->Project.Materials)
    {
        std::shared_ptr<unlitMaterial> UnlitMaterial = std::dynamic_pointer_cast<unlitMaterial>(Material.second);
        if(UnlitMaterial)
        {
            if(UnlitMaterial->BaseColorTexture.get() == Texture.get())
            {
                UnlitMaterial->SetBaseColorTexture(defaultTextures::BlackTexture);
            }
            if(UnlitMaterial->MetallicRoughnessTexture.get() == Texture.get())
            {
                UnlitMaterial->SetMetallicRoughnessTexture(defaultTextures::BlackTexture);
            }
            if(UnlitMaterial->NormalTexture.get() == Texture.get())
            {
                UnlitMaterial->SetNormalTexture(defaultTextures::BlackTexture);
            }
            if(UnlitMaterial->OcclusionTexture.get() == Texture.get())
            {
                UnlitMaterial->SetOcclusionTexture(defaultTextures::BlackTexture);
            }
            if(UnlitMaterial->EmissiveTexture.get() == Texture.get())
            {
                UnlitMaterial->SetEmissiveTexture(defaultTextures::BlackTexture);
            }
        }
    }
    
}

void context::AddMaterialToProject(std::shared_ptr<material> Material)
{
    if(this->Project.Materials.find(Material->UUID) != this->Project.Materials.end()) return;
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

    this->Project.Materials[Material->UUID] = Material;
}

void context::RemoveMaterialFromProject(std::shared_ptr<material> Material)
{
    //remove from materiails
    this->Project.Materials.erase(Material->UUID);
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
}

void context::AddGeometryToProject(std::shared_ptr<indexedGeometryBuffers> Geometry)
{
    if(this->Project.Geometries.find(Geometry->UUID) != this->Project.Geometries.end()) return;

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

    this->Project.Geometries[Geometry->UUID] = Geometry;
}

void context::AddMeshToProject(std::shared_ptr<mesh> Mesh)
{
    AddMaterialToProject(Mesh->Material);
    AddGeometryToProject(Mesh->GeometryBuffers);
    for(auto &Texture : Mesh->Material->AllTextures)
    {
        AddTextureToProject(Texture.second);
    }
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
            if(ProjectScene.second->Name == Scene->Name)
            {
                Scene->Name = BaseName + "_" + std::to_string(Count);
                OK = false;
                Count++;
            }
        }

        if(OK) break;
    }
    this->Project.Scenes[Scene->UUID] = Scene;
    this->Scene = Scene;
}

void context::RemoveObjectFromProject(std::shared_ptr<object3D> Object)
{
    this->Project.Objects.erase(Object->UUID);
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
                if(ProjectObject.second->Name == Object->Name)
                {
                    Object->Name = BaseName + "_" + std::to_string(Count);
                    OK = false;
                    Count++;
                }
            }

            if(OK) break;
        }
        this->Project.Objects[Object->UUID] = (Object);
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

void context::Update(std::shared_ptr<camera> Camera)
{

    Camera->Controls->OnUpdate();
    this->Scene->OnUpdate();
    this->Scene->OnBeforeRender(Camera);
    this->Scene->OnRender(Camera);
    this->Scene->OnAfterRender(Camera);
}

void context::EndFrame()
{
    std::shared_ptr<gfx::commandBuffer> CommandBuffer = GfxContext->GetCurrentFrameCommandBuffer();    
    Imgui->EndFrame(CommandBuffer);
    CommandBuffer->EndPass();
    GfxContext->EndFrame();
    GfxContext->Present();   
     
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


void context::Cleanup()
{
    GfxContext->WaitIdle();

    //Clean project
    this->Project.Geometries.clear();
    this->Project.Materials.clear();
    this->Project.Objects.clear();
    this->Project.Textures.clear();
    this->Project.Scenes.clear();
    this->Scene = nullptr;

    

    
    NoMaterial = nullptr;

    GfxContext->QueueDestroyImage(defaultTextures::BlackTexture->Handle);
    GfxContext->QueueDestroyImage(defaultTextures::WhiteTexture->Handle);
    GfxContext->QueueDestroyImage(defaultTextures::BlueTexture->Handle);
    
    GfxContext->ProcessDeletionQueue();

    for(auto &Pipeline : this->AllPipelines)
    {
        GfxContext->DestroyPipeline(Pipeline.second);
    }
    for(auto &Pipeline : this->Pipelines)
    {
        GfxContext->DestroyPipeline(Pipeline.second);
    }

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
        Project.Textures[Texture->UUID] = Texture;   
    }

    for (sz i = 0; i < MaterialFiles.size(); i++)
    {
        std::shared_ptr<material> Material = material::Deserialize(MaterialFiles[i]);
        Project.Materials[Material->UUID] = Material;   
    }
    
    for (sz i = 0; i < GeometryFiles.size(); i++)
    {
        std::shared_ptr<indexedGeometryBuffers> Geometry = indexedGeometryBuffers::Deserialize(GeometryFiles[i]);
        Project.Geometries[Geometry->UUID] = Geometry;   
    }
    
    for (sz i = 0; i < ObjectFiles.size(); i++)
    {
        std::shared_ptr<object3D> Object = object3D::Deserialize(ObjectFiles[i]);
        Project.Objects[Object->UUID] = Object;   
    }
    
    for (sz i = 0; i < SceneFiles.size(); i++)
    {
        std::shared_ptr<scene> Scene = std::static_pointer_cast<scene>(object3D::Deserialize(SceneFiles[i]));
        Project.Scenes[Scene->UUID] = Scene;  
        if(i==0) this->Scene = Scene;
    }
#if 0
#endif
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
        std::string FileName = Material.second->Name + ".mat";
        Material.second->Serialize(FolderName + "/" + FileName);
        AllFiles.push_back(FileName);
    }
    
    //Save all the textures
    for (auto &Texture : Project.Textures)
    {
        std::string FileName = Texture.second->Name + ".tex";
        Texture.second->Serialize(FolderName + "/" + FileName);
        AllFiles.push_back(FileName);
    }

    //Save all the geometries
    for (auto &Geometry : Project.Geometries)
    {
        std::string FileName = Geometry.second->Name + ".geom";
        Geometry.second->Serialize(FolderName + "/" + FileName);
        AllFiles.push_back(FileName);
    }

    //Save all the objects
    for (auto &Object : Project.Objects)
    {
        std::string FileName = Object.second->Name + ".obj";
        Object.second->Serialize(FolderName + "/" + FileName);
        AllFiles.push_back(FileName);
    }

    //Save all the scenes
    for (auto &Scene : Project.Scenes)
    {
        std::string FileName = Scene.second->Name + ".scene";
        Scene.second->Serialize(FolderName + "/" +FileName) ;
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