#include "Include/Light.h"
#include <glm/ext.hpp>

namespace hlgfx
{

light::light(std::string Name) : object3D(Name)
{
    this->Data.Color = v3f(1,1,1);
    this->Data.Intensity = 10.0f;
    this->Data.Size = v3f(1,1,1);
    this->Data.Position = this->Transform.GetWorldPosition();
    this->Data.Direction = glm::column(this->Transform.Matrices.LocalToWorld, 2);
}

void light::OnUpdate()
{
    this->Data.Position = this->Transform.GetWorldPosition();
    this->Data.Direction = glm::column(this->Transform.Matrices.LocalToWorld, 2);
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