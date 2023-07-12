#include "../Include/Image.h"
#include "GLImage.h"
#include "GLMapping.h"
#include <GL/glew.h>

namespace gfx
{

void image::Init(const imageData &ImageData, const imageCreateInfo &CreateInfo)
{
    this->Extent.Width = ImageData.Width;
    this->Extent.Height = ImageData.Height;
    this->Format = ImageData.Format;
    
    this->MipLevelCount = CreateInfo.GenerateMipmaps ? static_cast<u32>(std::floor(std::log2((std::max)(this->Extent.Width, this->Extent.Height)))) + 1 : 1;

    this->ApiData = std::make_shared<glImage>();
    std::shared_ptr<glImage> GLImage = std::static_pointer_cast<glImage>(this->ApiData);

    glGenTextures(1, &GLImage->Handle);
    glBindTexture(GL_TEXTURE_2D, GLImage->Handle);

    glTexImage2D(GL_TEXTURE_2D, 0, FormatToNativeInternal(ImageData.Format), ImageData.Width, ImageData.Height, 0, FormatToNative(ImageData.Format), TypeToNative(ImageData.Type), ImageData.Data);

    //TODO
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, SamplerWrapToNative(CreateInfo.WrapS));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, SamplerWrapToNative(CreateInfo.WrapT));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, SamplerWrapToNative(CreateInfo.WrapR));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, SamplerFilterToNative(CreateInfo.MinFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, SamplerFilterToNative(CreateInfo.MagFilter));
    
    if(CreateInfo.GenerateMipmaps) glGenerateMipmap(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}
}
