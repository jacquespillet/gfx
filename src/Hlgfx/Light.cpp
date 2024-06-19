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
        FramebufferCreateInfo.SetSize(context::ShadowMapSize, context::ShadowMapSize)
                                .AddColorFormat(gfx::format::R8G8B8A8_UNORM)  
                                .SetDepthFormat(context::ShadowMapFormat)
                                .SetClearColor(0, 0, 0, 0);
        this->ShadowsFramebuffer = gfx::context::Get()->CreateFramebuffer(FramebufferCreateInfo);
        this->PipelineHandleOffscreen = gfx::context::Get()->CreatePipelineFromFile("resources/Hlgfx/Shaders/ShadowMaps/ShadowMaps.json", this->ShadowsFramebuffer);
        this->Material = std::make_shared<customMaterial>("ShadowMaterial", this->PipelineHandleOffscreen);    
        this->ShadowCam = std::make_shared<camera>(-10, 10, -10, 10, 0.1, 100, true);
    }
}

void light::OnUpdate()
{
    v3f Pos =  this->Transform.GetWorldPosition();
    this->Data.Position[0] = Pos.x; this->Data.Position[1] = Pos.y; this->Data.Position[2] = Pos.z;  
    v3f Dir = glm::column(this->Transform.Matrices.LocalToWorld, 2);
    this->Data.Direction[0] = Dir.x; this->Data.Direction[1] = Dir.y; this->Data.Direction[2] = Dir.z;    
}

v4f calculateLightSpaceFrustumCorner(v3f startPoint, v3f direction,float width, m4x4 &LightViewMatrix) {
    v3f point = startPoint + direction * width;
    v4f point4f = v4f(point, 1.0f);
    point4f = LightViewMatrix * point4f;
    return point4f;
}

void light::CalculateMatrices(std::shared_ptr<camera> Camera)
{
    // Calculate frustum corners from camera
    float FinalShadowDistance = UseMainCameraFarPlane ? Camera->Data.FarClip : ShadowDistance;
    // float ShadowDistance = Camera->Data.FarClip;
    m4x4 &View = Camera->Data.ViewMatrix;
    m4x4 &Proj = Camera->Data.ProjectionMatrix;
    m4x4 InvV = glm::inverse(View);
    m4x4 InvP = glm::inverse(glm::perspective(glm::radians(Camera->Data.FOV), Camera->Data.AspectRatio, Camera->Data.NearClip, FinalShadowDistance));

    if(AutomaticShadowFrustum)
    {

        static std::array<v3f, 8> WorldSpaceCorners;
        static std::array<v3f, 8> LightSpaceCorners;

        int i=0;

        v3f Min(1e30f);
        v3f Max(-1e30f);
        for (unsigned int x = 0; x < 2; ++x)
        {
            for (unsigned int y = 0; y < 2; ++y)
            {
                for (unsigned int z = 0; z < 2; ++z)
                {
                    // Worl space point
                    v4f PointLightSpace =
                        InvP * v4f(
                            2.0f * x - 1.0f,
                            2.0f * y - 1.0f,
                            2.0f * z - 1.0f,
                            1.0f);
                    PointLightSpace /= PointLightSpace.w;
                    v4f PointWorldSpace = InvV * PointLightSpace;
                    WorldSpaceCorners[i++]=(PointWorldSpace);
                }
            }
        }

        // Compute Center in world space
        v3f Center(0);
        for (auto &Corner : WorldSpaceCorners) {
            Center += Corner;
        }
        Center /= 8.0f;

        // Get light direction
        v3f LightDir = -glm::column(this->Transform.Matrices.LocalToWorld, 2);
        // Shift the light position to the center
        this->Transform.Matrices.WorldToLocal = glm::lookAt(Center + LightDir, Center, v3f(0,1,0));
        

        for(int i=0; i<8; i++)
        {
            //Light space point 
            v4f LightSpacePoint = this->Transform.Matrices.WorldToLocal * v4f(WorldSpaceCorners[i], 1);

            // Calculate Min/Max
            Min = glm::min(Min, v3f(LightSpacePoint));
            Max = glm::max(Max, v3f(LightSpacePoint));        
        }


        // this->ShadowCam->SetOrtho(Min.x, Max.x, Min.y, Max.y, -100, 100);

        // Set the camera projection matrix
        this->ShadowCam->Transform.Matrices.WorldToLocal = this->Transform.Matrices.WorldToLocal;
    
        this->ShadowCam->SetOrtho(Min.x, Max.x, Min.y, Max.y, Min.z, Max.z);

        this->Data.LightSpaceMatrix = this->ShadowCam->Data.ViewProjectionMatrix;
    }
    else
    {
        // this->ShadowCam->SetOrtho(Min.x, Max.x, Min.y, Max.y, -100, 100);

        // Set the camera projection matrix
        v3f Center = this->Transform.GetWorldPosition();
        // Get light direction
        v3f LightDir = -glm::column(this->Transform.Matrices.LocalToWorld, 2);
        // Shift the light position to the center
        this->Transform.Matrices.WorldToLocal = glm::lookAt(Center + LightDir, Center, v3f(0,1,0));

        this->ShadowCam->Transform.Matrices.WorldToLocal = this->Transform.Matrices.WorldToLocal;
        this->ShadowCam->SetOrtho(-ShadowFrustumSize.x * 0.5f, ShadowFrustumSize.x * 0.5f,
                                  -ShadowFrustumSize.y * 0.5f, ShadowFrustumSize.y * 0.5f, 
                                  -ShadowFrustumSize.z * 0.5f, ShadowFrustumSize.z * 0.5f);
        this->Data.LightSpaceMatrix = this->ShadowCam->Data.ViewProjectionMatrix;
    }
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