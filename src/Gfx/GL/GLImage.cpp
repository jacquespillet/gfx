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
    
    this->MipLevelCount = CreateInfo._GenerateMipmaps ? static_cast<u32>(std::floor(std::log2((std::max)(this->Extent.Width, this->Extent.Height)))) + 1 : 1;

    this->ApiData = std::make_shared<glImage>();
    std::shared_ptr<glImage> GLImage = std::static_pointer_cast<glImage>(this->ApiData);

    glGenTextures(1, &GLImage->Handle);
    glBindTexture(GL_TEXTURE_2D, GLImage->Handle);

    glTexImage2D(GL_TEXTURE_2D, 0, FormatToNativeInternal(ImageData.Format), ImageData.Width, ImageData.Height, 0, FormatToNative(ImageData.Format), TypeToNative(ImageData.Type), ImageData.Data);

    //TODO
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, Mapping(CreateInfo._WrapS));
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, Mapping(CreateInfo._WrapT));
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, Mapping(CreateInfo._WrapR));
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, Mapping(CreateInfo._MinFilter));
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, Mapping(CreateInfo._MaxFilter));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_REPEAT); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); 
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &CreateInfo._BorderColor[0]);
    
    if(CreateInfo._GenerateMipmaps) glGenerateMipmap(GL_TEXTURE_2D);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}
}
