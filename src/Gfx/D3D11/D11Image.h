#pragma once
#include <d3d11_1.h>

namespace gfx
{
struct d3d11Image
{
    ID3D11Texture2D* Handle;
    ID3D11ShaderResourceView* View;

};
}