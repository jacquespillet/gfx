#include "resources/Shaders/Common/Macros.hlsl"

struct PSInput
{
    vec4 position : SV_POSITION;
    vec4 color : COLOR;
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

cbuffer MainConstantBuffer4 : register(b3)
{
    vec4 Color6;
    vec4 Color7;
};

Texture2D color : register(t4);
SamplerState defaultSampler : register(s0);


PSInput VSMain(vec4 position : POSITION0, vec4 color : POSITION1, vec2 instancePosition : TEXCOORD2)
{
    PSInput result;

    result.uv = position.xy;
    result.position = position + vec4(instancePosition, 0, 0);
    result.color = color;

    return result;
}

vec4 PSMain(PSInput input) : SV_TARGET
{
    return SampleTexture(color, defaultSampler, input.uv) + input.color + Color0;
}
