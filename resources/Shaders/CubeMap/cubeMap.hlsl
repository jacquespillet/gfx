#include "resources/Shaders/Common/Macros.hlsl"

struct PSInput
{
    vec4 position : SV_POSITION;
    vec4 cubemapPosition : NORMAL;
    vec2 uv : TEXCOORD;
};

cbuffer MainConstantBuffer4 : register(b5)
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

TextureCube color : register(t4);
SamplerState defaultSampler : register(s0);


PSInput VSMain(vec4 position : POSITION0, vec2 uv : TEXCOORD1)
{
    PSInput Output;

    mat4 VP = mul(ProjectionMatrix, ViewMatrix);
    Output.position = mul(VP, position);
    Output.cubemapPosition = position;
    Output.uv = uv;

    return Output;
}

vec4 PSMain(PSInput input) : SV_TARGET
{
    return SampleTexture(color, defaultSampler, normalize(input.cubemapPosition.xyz));
}
