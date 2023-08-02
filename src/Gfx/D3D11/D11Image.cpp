#include "../Include/Image.h"
#include "../Include/Context.h"
#include "../Common/Util.h"
#include "D11Image.h"
#include "D11Common.h"
#include "D11Context.h"
#include <d3d11_1.h>

namespace gfx
{

void image::Init(const imageData &ImageData, const imageCreateInfo &CreateInfo)
{
    GET_CONTEXT(D11Data, context::Get());

    this->Extent.Width = ImageData.Width;
    this->Extent.Height = ImageData.Height;
    this->Format = ImageData.Format;
    this->ChannelCount = ImageData.ChannelCount;
    this->Data.resize(ImageData.Width * ImageData.Height * FormatSize(Format));
    this->Type = ImageData.Type;

    memcpy(this->Data.data(), ImageData.Data, ImageData.DataSize);

    
    this->MipLevelCount = CreateInfo.GenerateMipmaps ? static_cast<u32>(std::floor(std::log2((std::max)(this->Extent.Width, this->Extent.Height)))) + 1 : 1;

    this->ApiData = std::make_shared<d3d11Image>();
    GET_API_DATA(D11Image, d3d11Image, this);

    //TODO
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width              = ImageData.Width;  // in xdata.h
    TextureDesc.Height             = ImageData.Height; // in xdata.h
    TextureDesc.MipLevels          = 1;
    TextureDesc.ArraySize          = 1;
    TextureDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    TextureDesc.SampleDesc.Count   = 1;
    TextureDesc.Usage              = D3D11_USAGE_IMMUTABLE;
    TextureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA TextureData = {};
    TextureData.pSysMem            = ImageData.Data;
    TextureData.SysMemPitch        = ImageData.Width * FormatSize(Format); // 4 bytes per pixel

    D11Data->Device->CreateTexture2D(&TextureDesc, &TextureData, &D11Image->Handle);
    D11Data->Device->CreateShaderResourceView(D11Image->Handle, nullptr, &D11Image->View);

    //TODO: Mipmaps
}

}