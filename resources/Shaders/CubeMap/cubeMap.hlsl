#include "resources/Shaders/Common/Macros.hlsl"

struct PSInput
{
    vec4 position : SV_POSITION;
    vec2 uv : TEXCOORD;
};

cbuffer MainConstantBuffer1 : register(b0)
{
    vec4 Color0;
    vec4 Color1;
};

cbuffer MainConstantBuffer2 : register(b1)
{
    vec4 Color2;
    vec4 Color3;
};

cbuffer MainConstantBuffer3 : register(b2)
{
    vec4 Color4;
    vec4 Color5;
};

cbuffer MainConstantBuffer4 : register(b5)
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

Texture2D color : register(t4);
SamplerState defaultSampler : register(s0);


PSInput VSMain(vec4 position : POSITION0, vec2 uv : TEXCOORD1)
{
    PSInput Output;

    mat4 VP = mul(ProjectionMatrix, ViewMatrix);
    Output.position = mul(VP, position);
    Output.uv = uv;

    return Output;
}

vec4 PSMain(PSInput input) : SV_TARGET
{
    return SampleTexture(color, defaultSampler, input.uv);
}
