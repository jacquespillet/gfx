#include "Include/Light.h"
#include "Include/Material.h"
#include "Include/Context.h"
#include "Gfx/Include/Context.h"
#include <glm/ext.hpp>

namespace hlgfx
{

light::light(std::string Name, lightType Type) : object3D(Name)
{
    this->Data.ColorAndIntensity.r = 1; this->Data.ColorAndIntensity.g = 1; this->Data.ColorAndIntensity.b = 1;
    this->Data.ColorAndIntensity.w = 10.0f;
    this->Data.SizeAndType.x = 1; this->Data.SizeAndType.y = 1; this->Data.SizeAndType.z = 1;
    this->Data.SizeAndType.z = (f32)Type;
    
    v3f Pos =  this->Transform.GetWorldPosition();
    this->Data.Position[0] = Pos.x; this->Data.Position[1] = Pos.y; this->Data.Position[2] = Pos.z;  
    
    v3f Dir = glm::column(this->Transform.Matrices.LocalToWorld, 2);
    this->Data.Direction[0] = Dir.x; this->Data.Direction[1] = Dir.y; this->Data.Direction[2] = Dir.z;

    if(Type == lightType::Directional)
    {
        gfx::framebufferCreateInfo FramebufferCreateInfo = {};
        FramebufferCreateInfo.SetSize(1024, 1024)
                                .AddColorFormat(gfx::format::R8G8B8A8_UNORM)  
                                .SetDepthFormat(gfx::format::D16_UNORM)
                                .SetClearColor(0, 0, 0, 0);
        this->ShadowsFramebuffer = gfx::context::Get()->CreateFramebuffer(FramebufferCreateInfo);
        this->PipelineHandleOffscreen = gfx::context::Get()->CreatePipelineFromFile("resources/Hlgfx/Shaders/ShadowMaps/ShadowMaps.json", this->ShadowsFramebuffer);
        this->Material = std::make_shared<customMaterial>("ShadowMaterial", this->PipelineHandleOffscreen);    
        this->ShadowCam = std::make_shared<camera>(-10, 10, -10, 10, 0.1, 100);
    }
}

void light::OnUpdate()
{
    v3f Pos =  this->Transform.GetWorldPosition();
    this->Data.Position[0] = Pos.x; this->Data.Position[1] = Pos.y; this->Data.Position[2] = Pos.z;  
    v3f Dir = glm::column(this->Transform.Matrices.LocalToWorld, 2);
    this->Data.Direction[0] = Dir.x; this->Data.Direction[1] = Dir.y; this->Data.Direction[2] = Dir.z;    
}

v4f light::CalculateLightSpaceFrustumCorner(v3f StartPoint, v3f Direction, f32 Width)
{
    v3f Point = StartPoint + Direction * Width;
    return this->Transform.Matrices.WorldToLocal * v4f(Point, 1);
}

void light::UpdateViewBox(std::shared_ptr<camera> Camera)
{
    FarWidth = ShadowDistance * tan(glm::radians(Camera->Data.FOV));
    NearWidth = Camera->Data.NearClip * tan(glm::radians(Camera->Data.FOV));
    FarHeight = FarWidth / Camera->Data.AspectRatio;
    NearHeight = NearWidth / Camera->Data.AspectRatio;  

    m4x4 RotationMatrix = Camera->Transform.Matrices.LocalToWorld;
    v3f ForwardVector = glm::column(RotationMatrix, 2);

    v3f ToFar = ForwardVector;
    ToFar *= ShadowDistance;
    v3f ToNear = ForwardVector;
    ToNear *= Camera->Data.NearClip;

    v3f CenterNear = this->ShadowCam->Transform.GetWorldPosition() + ToNear;
    v3f CenterFar = this->ShadowCam->Transform.GetWorldPosition() + ToFar;

    static v3f Points[8];
    
    v3f UpVector = glm::column(RotationMatrix, 1);
    v3f DownVector = -UpVector;
    v3f RightVector = glm::column(RotationMatrix, 0);
    v3f LeftVector = -RightVector;

    v3f FarTop = CenterFar + UpVector * FarHeight;
    v3f FarBottom = CenterFar + DownVector * FarHeight;
    v3f NearTop = CenterNear + UpVector * NearHeight;
    v3f NearBottom = CenterNear + DownVector * NearHeight;

    Points[0] = CalculateLightSpaceFrustumCorner(FarTop, RightVector, FarWidth);
    Points[1] = CalculateLightSpaceFrustumCorner(FarTop, LeftVector, FarWidth);
    Points[2] = CalculateLightSpaceFrustumCorner(FarBottom, RightVector, FarWidth);
    Points[3] = CalculateLightSpaceFrustumCorner(FarBottom, LeftVector, FarWidth);
    Points[4] = CalculateLightSpaceFrustumCorner(NearTop, RightVector, NearWidth);
    Points[5] = CalculateLightSpaceFrustumCorner(NearTop, LeftVector, NearWidth);
    Points[6] = CalculateLightSpaceFrustumCorner(NearBottom, RightVector, NearWidth);
    Points[7] = CalculateLightSpaceFrustumCorner(NearBottom, LeftVector, NearWidth);



    BoxMin = v3f(1e30f);
    BoxMax = v3f(-1e30f);
    for (u8 i = 0; i < 8; i++)
    {
        BoxMin = glm::min(BoxMin, Points[i]);
        BoxMax = glm::max(BoxMax, Points[i]);
    }
}

void light::CalculateMatrices(std::shared_ptr<camera> Camera)
{
    this->ShadowCam->SetOrtho(-this->ShadowMapViewport.x * 0.5f, this->ShadowMapViewport.x * 0.5f,-this->ShadowMapViewport.y * 0.5f, this->ShadowMapViewport.y * 0.5f, 0, this->ShadowMapViewport.z);
    m4x4 LocalToWorldMatrix = this->Transform.Matrices.LocalToWorld;
    v3f CamPos = glm::column(LocalToWorldMatrix, 2) * ShadowMapDistance;
    LocalToWorldMatrix[3][0] = CamPos.x;
    LocalToWorldMatrix[3][1] = CamPos.y;
    LocalToWorldMatrix[3][2] = CamPos.z;
    this->ShadowCam->Transform.Matrices.WorldToLocal = glm::inverse(LocalToWorldMatrix);
    this->ShadowCam->RecalculateMatrices();
    this->Data.LightSpaceMatrix = this->ShadowCam->Data.ViewProjectionMatrix;
}

void light::Serialize(std::ofstream &FileStream)
{
    std::vector<u8> Result;

    u32 Object3DType;
    if(this->Data.SizeAndType.w == lightType::Directional) Object3DType = (u32) object3DType::Light_Directional;
    else if(this->Data.SizeAndType.w == lightType::Point) Object3DType = (u32) object3DType::Light_Point;

    FileStream.write((char*)&Object3DType, sizeof(u32));

    u32 UUIDSize = this->UUID.size();
    FileStream.write((char*)&UUIDSize, sizeof(u32));
    FileStream.write(this->UUID.data(), this->UUID.size());
    
    u32 StringLength = this->Name.size();
    FileStream.write((char*)&StringLength, sizeof(u32));
    FileStream.write((char*)(void*)this->Name.data(), StringLength);
    
    FileStream.write((char*)&this->Transform.Matrices, sizeof(transform::matrices));
    FileStream.write((char*)&this->Transform.LocalValues, sizeof(transform::localValues));
    
    FileStream.write((char*)&this->Data, sizeof(lightData));
    

    u32 NumChildren = this->Children.size();
    FileStream.write((char*)&NumChildren, sizeof(u32));

    for (sz i = 0; i < NumChildren; i++)
    {
        this->Children[i]->Serialize(FileStream);
    }
}

light::~light()
{
    printf("Destroying Light\n");
    gfx::context::Get()->DestroyPipeline(this->PipelineHandleOffscreen);
    gfx::context::Get()->DestroyFramebuffer(this->ShadowsFramebuffer);
    this->Material = nullptr;
    this->ShadowCam = nullptr;
}
}