#include "../Include/Image.h"
#include "../Include/Context.h"
#include "../Common/Util.h"
#include "D11Image.h"
#include "D11Common.h"
#include "D11Context.h"
#include "D11Mapping.h"
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
    memcpy(Data.data(), ImageData.Data, Data.size());
    this->Type = ImageData.Type;
    this->CreateInfo = CreateInfo;

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
    TextureDesc.Format             = FormatToNative(ImageData.Format);
    TextureDesc.SampleDesc.Count   = 1;
    TextureDesc.Usage              = D3D11_USAGE_IMMUTABLE;
    TextureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA TextureData = {};
    TextureData.pSysMem            = ImageData.Data;
    TextureData.SysMemPitch        = ImageData.Width * FormatSize(Format); // 4 bytes per pixel

    D11Data->Device->CreateTexture2D(&TextureDesc, &TextureData, D11Image->Handle.GetAddressOf());
    D11Data->Device->CreateShaderResourceView(D11Image->Handle.Get(), nullptr, D11Image->View.GetAddressOf());

    //TODO: Mipmaps
}

void image::InitAsCubemap(const imageData &Left, const imageData &Right, const imageData &Top, const imageData &Bottom, const imageData &Back, const imageData &Front, const imageCreateInfo &CreateInfo)
{
    std::shared_ptr<d3d11Data> D11Data = std::static_pointer_cast<d3d11Data>(context::Get()->ApiContextData);

    Extent.Width = Left.Width;
    Extent.Height = Left.Height;
    Format = Left.Format;
    ByteSize = Left.DataSize;
    MipLevelCount = CreateInfo.GenerateMipmaps ? static_cast<u32>(std::floor(std::log2((std::max)(this->Extent.Width, this->Extent.Height)))) + 1 : 1;
    this->Data.resize(Left.DataSize + Right.DataSize + Top.DataSize + Bottom.DataSize + Back.DataSize + Front.DataSize);
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
    this->ChannelCount = Left.ChannelCount;
    this->Type = Left.Type;


    this->ApiData = std::make_shared<d3d11Image>();
    GET_API_DATA(D11Image, d3d11Image, this);

    D3D11_TEXTURE2D_DESC cubemapDesc;
    ZeroMemory(&cubemapDesc, sizeof(cubemapDesc));
    cubemapDesc.Width = Left.Width;                   // Width of each face
    cubemapDesc.Height = Left.Height;                  // Height of each face
    cubemapDesc.MipLevels = 1;
    cubemapDesc.ArraySize = 6;                  // Number of faces
    cubemapDesc.Format = FormatToNative(Left.Format); // Choose the appropriate format
    cubemapDesc.SampleDesc.Count = 1;
    cubemapDesc.Usage = D3D11_USAGE_DEFAULT;
    cubemapDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;
    cubemapDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    D11Data->Device->CreateTexture2D(&cubemapDesc, nullptr, D11Image->Handle.GetAddressOf());


    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    ZeroMemory(&srvDesc, sizeof(srvDesc));
    srvDesc.Format = cubemapDesc.Format;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
    srvDesc.TextureCube.MostDetailedMip = 0;
    srvDesc.TextureCube.MipLevels = 1;
    D11Data->Device->CreateShaderResourceView(D11Image->Handle.Get(), &srvDesc, D11Image->View.GetAddressOf());    

    const std::vector<std::reference_wrapper<const imageData>> Images = {Right, Left, Top, Bottom, Front, Back};
    for (int face = 0; face < 6; ++face)
    {
        D3D11_BOX box;
        box.left = 0;
        box.right = Images[face].get().Width;
        box.top = 0;
        box.bottom = Images[face].get().Height;
        box.front = 0;
        box.back = 1;

        // UpdateSubresource the cubemap face with the corresponding image data
        u32 DestSubresource = D3D11CalcSubresource(0, face, 1);
        void *Data = Images[face].get().Data;
        sz Pitch = Images[face].get().DataSize / Images[face].get().Height;
        D11Data->DeviceContext->UpdateSubresource(D11Image->Handle.Get(), DestSubresource, &box, Data, Pitch, 0);
    }
}


void image::InitAsArray(u32 Width, u32 Height, u32 Depth, format Format, imageUsage::value ImageUsage, memoryUsage MemoryUsage, u32 SampleCount)
{
    std::shared_ptr<d3d11Data> D11Data = std::static_pointer_cast<d3d11Data>(context::Get()->ApiContextData);

    this->Extent.Width = Width;
    this->Extent.Height = Height;
    this->LayerCount = Depth;
    this->Format = Format;
    this->MipLevelCount = 1;
    this->ChannelCount = ChannelCountFromFormat(Format);
    this->Type = type::BYTE;
    this->Data.resize(Width * Height * Depth * FormatSize(Format));


    ApiData = std::make_shared<d3d11Image>();
    std::shared_ptr<d3d11Image> D11Image = std::static_pointer_cast<d3d11Image>(ApiData);

   //TODO
    D3D11_TEXTURE2D_DESC TextureDesc = {};
    TextureDesc.Width              = Width;  // in xdata.h
    TextureDesc.Height             = Height; // in xdata.h
    TextureDesc.MipLevels          = 1;
    TextureDesc.ArraySize          = Depth;
    TextureDesc.Format             = FormatToNative(Format);
    if(Format == format::D16_UNORM) TextureDesc.Format = DXGI_FORMAT_R16_UNORM;
    TextureDesc.SampleDesc.Count   = 1;
    TextureDesc.Usage              = D3D11_USAGE_DEFAULT;
    TextureDesc.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

    D11Data->Device->CreateTexture2D(&TextureDesc, nullptr, D11Image->Handle.GetAddressOf());
    D11Data->Device->CreateShaderResourceView(D11Image->Handle.Get(), nullptr, D11Image->View.GetAddressOf());        

}

ImTextureID image::GetImGuiID()
{
    GET_API_DATA(D11Image, d3d11Image, this);
    return (ImTextureID) D11Image->View.Get();
}


}