#include "resources/Hlgfx/Shaders/Common/Macros.hlsl"
#include "resources/Hlgfx/Shaders/Common/Bindings.h"

struct PSInput
{
    vec4 position : SV_POSITION;
    vec2 uv : TEXCOORD;
};

cbuffer Camera : register(b0)
{
    float FOV;
    float AspectRatio;
    float NearClip;
    float FarClip;

    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
    mat4 ViewProjectionMatrix;    
};

cbuffer Model : register(b1)
{
    mat4 ModelMatrix;    
};

cbuffer Material : register(b3)
{
    vec4 BaseColor;
};

Texture2D DiffuseTexture : register(t4);
SamplerState DefaultSampler : register(s0);

PSInput VSMain(vec4 PositionUvX : POSITION0, vec4 NormalUvY : POSITION1)
{
    PSInput result;

    result.uv = vec2(PositionUvX.w, NormalUvY.w);
    result.position = mul(mul(ViewProjectionMatrix, ModelMatrix), vec4(PositionUvX.xyz, 1));

    return result;
}

vec4 PSMain(PSInput Input) : SV_TARGET
{
    return BaseColor + SampleTexture(DiffuseTexture, DefaultSampler, Input.uv);
}
