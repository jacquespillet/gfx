#include "Include/Light.h"
#include "Include/Material.h"
#include "Include/Context.h"
#include "Gfx/Include/Context.h"
#include <glm/ext.hpp>

namespace hlgfx
{

light::light(std::string Name) : object3D(Name)
{
    this->Data.ColorAndIntensity.r = 1; this->Data.ColorAndIntensity.g = 1; this->Data.ColorAndIntensity.b = 1;
    this->Data.ColorAndIntensity.w = 10.0f;
    this->Data.SizeAndType.x = 1; this->Data.SizeAndType.y = 1; this->Data.SizeAndType.z = 1;
    v3f Pos =  this->Transform.GetWorldPosition();
    this->Data.Position[0] = Pos.x; this->Data.Position[1] = Pos.y; this->Data.Position[2] = Pos.z;  
    v3f Dir = glm::column(this->Transform.Matrices.LocalToWorld, 2);
    this->Data.Direction[0] = Dir.x; this->Data.Direction[1] = Dir.y; this->Data.Direction[2] = Dir.z;


	gfx::framebufferCreateInfo FramebufferCreateInfo = {};
    FramebufferCreateInfo.SetSize(1024, 1024)
                            .AddColorFormat(gfx::format::R8G8B8A8_UNORM)
                            .SetDepthFormat(gfx::format::D16_UNORM)
                            .SetClearColor(0, 0, 0, 0);
    this->ShadowsFramebuffer = gfx::context::Get()->CreateFramebuffer(FramebufferCreateInfo);
    this->PipelineHandleOffscreen = gfx::context::Get()->CreatePipelineFromFile("resources/Hlgfx/Shaders/ShadowMaps/ShadowMaps.json", this->ShadowsFramebuffer);
    this->Material = std::make_shared<customMaterial>("ShadowMaterial", this->PipelineHandleOffscreen);    
}

void light::OnUpdate()
{
    v3f Pos =  this->Transform.GetWorldPosition();
    this->Data.Position[0] = Pos.x; this->Data.Position[1] = Pos.y; this->Data.Position[2] = Pos.z;  
    v3f Dir = glm::column(this->Transform.Matrices.LocalToWorld, 2);
    this->Data.Direction[0] = Dir.x; this->Data.Direction[1] = Dir.y; this->Data.Direction[2] = Dir.z;    
}

void light::Serialize(std::ofstream &FileStream)
{
    std::vector<u8> Result;

    u32 Object3DType = (u32) object3DType::Light;
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
}