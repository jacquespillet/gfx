#include "resources/Shaders/Common/Macros.hlsl"

struct PSInput
{
    vec4 position : SV_POSITION;
    vec4 color : COLOR;
    vec2 uv : TEXCOORD;
};

Texture2D color : register(t4);
SamplerState defaultSampler : register(s0);


PSInput VSMain(vec4 position : POSITION0, vec4 color : POSITION1)
{
    PSInput result;

    result.uv = position.xy;
    result.position = position;
    result.color = color;

    return result;
}

vec4 PSMain(PSInput input) : SV_TARGET
{
    return SampleTexture(color, defaultSampler, input.uv);
}
