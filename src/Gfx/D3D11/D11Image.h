#pragma once
#include <d3d11_1.h>

#include <wrl.h>
using namespace Microsoft::WRL;

namespace gfx
{
struct d3d11Image
{
    ComPtr<ID3D11Texture2D> Handle = nullptr;
    ComPtr<ID3D11ShaderResourceView> View = nullptr;
};
}