#include "../Include/Image.h"
#include "GLImage.h"
#include "GLMapping.h"
#include <glad/gl.h>

namespace gfx
{

void image::Init(imageData &ImageData, const imageCreateInfo &CreateInfo)
{
    this->Extent.Width = ImageData.Width;
    this->Extent.Height = ImageData.Height;
    this->Format = ImageData.Format;
    this->ChannelCount = ImageData.ChannelCount;
    this->Data.resize(ImageData.Width * ImageData.Height * FormatSize(Format));
    memcpy(Data.data(), ImageData.Data, Data.size());
    this->Type = ImageData.Type;
    this->CreateInfo = CreateInfo;

    memcpy(this->Data.data(), ImageData.Data, ImageData.DataSize);

    
    this->MipLevelCount = CreateInfo.GenerateMipmaps ? static_cast<u32>(std::floor(std::log2((std::max)(this->Extent.Width, this->Extent.Height)))) + 1 : 1;

    this->ApiData = std::make_shared<glImage>();
    GET_API_DATA(GLImage, glImage, this);
    glGenTextures(1, &GLImage->Handle);
    glBindTexture(GL_TEXTURE_2D, GLImage->Handle);

    // glTexImage2D(GL_TEXTURE_2D, 0, FormatToNativeInternal(ImageData.Format), ImageData.Width, ImageData.Height, 0, FormatToNative(ImageData.Format), TypeToNative(ImageData.Type), ImageData.Data);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, ImageData.Width, ImageData.Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ImageData.Data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, SamplerWrapToNative(CreateInfo.WrapS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, SamplerWrapToNative(CreateInfo.WrapT));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, SamplerWrapToNative(CreateInfo.WrapR));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, SamplerMinFilterToNative(CreateInfo.MinFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, SamplerMagFilterToNative(CreateInfo.MagFilter));

    
    if(IsDepthFormat(Format))
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE_ARB);
    }    
    
    if(CreateInfo.GenerateMipmaps) glGenerateMipmap(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void image::InitAsArray(u32 Width, u32 Height, u32 Depth, format Format, imageUsage::value ImageUsage, memoryUsage MemoryUsage, u32 SampleCount)
{
    this->Extent.Width = Width;
    this->Extent.Height = Height;
    this->Format = Format;
    this->ChannelCount = ChannelCountFromFormat(Format);
    this->Data.resize(Width * Height * FormatSize(Format));
    // this->Type = FormatToType(Format);
    this->MipLevelCount = 1;


    this->ApiData = std::make_shared<glImage>();
    GET_API_DATA(GLImage, glImage, this);
    glGenTextures(1, &GLImage->Handle);
    glBindTexture(GL_TEXTURE_2D_ARRAY, GLImage->Handle);

    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, FormatToNativeInternal(Format), Width, Height, Depth, 0, FormatToNative(Format), FormatToType(Format), nullptr);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
    if(IsDepthFormat(Format))
    {
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
        glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE_ARB);
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
}


void image::InitAsCubemap(const imageData &Left, const imageData &Right, const imageData &Top, const imageData &Bottom, const imageData &Back, const imageData &Front, const imageCreateInfo &CreateInfo)
{
    //Assume all images have same size and format
    this->Extent.Width = Left.Width;
    this->Extent.Height = Left.Height;
    this->Format = Left.Format;
    this->MipLevelCount = CreateInfo.GenerateMipmaps ? static_cast<u32>(std::floor(std::log2((std::max)(this->Extent.Width, this->Extent.Height)))) + 1 : 1;
    this->LayerCount = 6;
    this->ChannelCount = Left.ChannelCount;
    this->Data.resize(Left.DataSize + Right.DataSize + Top.DataSize + Bottom.DataSize + Back.DataSize + Front.DataSize);
    this->Type = Left.Type;
    sz Offset=0;
    memcpy(this->Data.data() + Offset, Left.Data, Left.DataSize);
    Offset += Left.DataSize;
    memcpy(this->Data.data() + Offset, Right.Data, Right.DataSize);
    Offset += Right.DataSize;
    memcpy(this->Data.data() + Offset, Top.Data, Top.DataSize);
    Offset += Top.DataSize;
    memcpy(this->Data.data() + Offset, Bottom.Data, Bottom.DataSize);
    Offset += Bottom.DataSize;
    memcpy(this->Data.data() + Offset, Back.Data, Back.DataSize);
    Offset += Back.DataSize;
    memcpy(this->Data.data() + Offset, Front.Data, Front.DataSize);
    Offset += Front.DataSize;


    this->ApiData = std::make_shared<glImage>();
    GET_API_DATA(GLImage, glImage, this);
    glGenTextures(1, &GLImage->Handle);
    glBindTexture(GL_TEXTURE_CUBE_MAP, GLImage->Handle);

    const std::vector<std::reference_wrapper<const imageData>> Images = {Right, Left, Top, Bottom, Front, Back};
    for(unsigned int i = 0; i < 6; i++)
    {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
            0, FormatToNativeInternal(this->Format), this->Extent.Width, this->Extent.Height, 0, FormatToNative(this->Format), TypeToNative(Images[i].get().Type), Images[i].get().Data
        );
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, SamplerWrapToNative(CreateInfo.WrapS));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, SamplerWrapToNative(CreateInfo.WrapT));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, SamplerWrapToNative(CreateInfo.WrapR));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, SamplerMinFilterToNative(CreateInfo.MinFilter));
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, SamplerMagFilterToNative(CreateInfo.MagFilter));

    
    if(IsDepthFormat(Format))
    {
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE_ARB);
    }

    if(CreateInfo.GenerateMipmaps) glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}


ImTextureID image::GetImGuiID()
{
    GET_API_DATA(GLImage, glImage, this);
    return (ImTextureID)GLImage->Handle;
}

}
