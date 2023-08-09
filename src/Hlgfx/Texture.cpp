#include "Include/Texture.h"
#include "Include/Context.h"
#include "gfx/Include/Context.h"

#include <fstream>

namespace hlgfx
{
texture::texture(std::string Name)
{
    this->Name = Name;
    this->Handle = gfx::InvalidHandle;
    this->UUID = context::Get()->GetUUID();
}

texture::texture(std::string Name, gfx::imageHandle Handle) : Handle(Handle)
{
    this->Name = Name;
    this->UUID = context::Get()->GetUUID();
}

void texture::Serialize(std::string FilePath)
{
    std::ofstream FileStream;
    FileStream.open(FilePath, std::ios::trunc | std::ios::binary);
    assert(FileStream.is_open());

    gfx::image *Image = gfx::context::Get()->GetImage(this->Handle);

    u32 UUIDSize = UUID.size();
    FileStream.write((char*)&UUIDSize, sizeof(u32));
    FileStream.write((char*)UUID.data(), UUID.size());
    
    u32 NameSize = Name.size();
    FileStream.write((char*)&NameSize, sizeof(u32));
    FileStream.write((char*)Name.data(), Name.size());

    FileStream.write((char*)&Image->Extent.Width, sizeof(u32));
    FileStream.write((char*)&Image->Extent.Height, sizeof(u32));
    FileStream.write((char*)&Image->Format, sizeof(u32));
    FileStream.write((char*)&Image->ChannelCount , sizeof(u8));
    FileStream.write((char*)&Image->Type , sizeof(gfx::type));
    sz Size = Image->Data.size();  
    FileStream.write((char*)&Size, sizeof(sz));
    FileStream.write((char*)Image->Data.data(), Image->Data.size());    
    FileStream.close();
}

std::shared_ptr<texture> texture::Clone()
{
    gfx::image *Image = gfx::context::Get()->GetImage(this->Handle);
    gfx::imageData ImageData = {};
    ImageData.ChannelCount = Image->ChannelCount;
    ImageData.Data = Image->Data.data();
    ImageData.DataSize = Image->Data.size();
    ImageData.Format = Image->Format;
    ImageData.Type = Image->Type;
    ImageData.Width = Image->Extent.Width;
    ImageData.Height = Image->Extent.Height;

    gfx::imageHandle DuplicatedImage = gfx::context::Get()->CreateImage(ImageData, Image->CreateInfo);
    std::shared_ptr<texture> Result = std::make_shared<texture>(this->Name + "_Duplicated", DuplicatedImage);
    return Result;
}

std::shared_ptr<texture> texture::Deserialize(std::string FilePath)
{
    std::ifstream FileStream;
    FileStream.open(FilePath, std::ios::binary);
    assert(FileStream.is_open());
    
    std::shared_ptr<texture> Result = std::make_shared<texture>();

    u32 UUIDSize;
    FileStream.read((char*)&UUIDSize, sizeof(u32));
    Result->UUID.resize(UUIDSize);
    FileStream.read(Result->UUID.data(), Result->UUID.size());

    u32 NameSize;
    FileStream.read((char*)&NameSize, sizeof(u32));
    Result->Name.resize(NameSize);
    FileStream.read(Result->Name.data(), Result->Name.size());

    gfx::imageData ImageData = {};
    FileStream.read((char*)&ImageData.Width, sizeof(u32));
    FileStream.read((char*)&ImageData.Height, sizeof(u32));
    FileStream.read((char*)&ImageData.Format, sizeof(u32));
    u8 ChannelCount=0;
    FileStream.read((char*)&ChannelCount, sizeof(u8));
    ImageData.ChannelCount = (s32)ChannelCount;
    FileStream.read((char*)&ImageData.Type, sizeof(gfx::type));
    
    FileStream.read((char*)&ImageData.DataSize, sizeof(sz));
    ImageData.Data = (u8*) gfx::AllocateMemory(ImageData.DataSize);
    FileStream.read((char*)ImageData.Data, ImageData.DataSize);
    

    FileStream.close();

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
    Result->Handle = gfx::context::Get()->CreateImage(ImageData, ImageCreateInfo);

    gfx::DeallocateMemory(ImageData.Data);
    return Result;
}

texture::~texture()
{
    gfx::context::Get()->QueueDestroyImage(this->Handle);
}
}