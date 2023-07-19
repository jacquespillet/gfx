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

RWStructuredBuffer<vec4> InstancePositions : register(u2);

Texture2D color : register(t4);
SamplerState defaultSampler : register(s0);


PSInput VSMain(vec4 position : POSITION0, vec4 color : POSITION1, uint InstanceID : SV_InstanceID)
{
    PSInput result;
    
    vec3 pos = position.xyz * 0.1;
    
    result.position = vec4(pos +  InstancePositions[InstanceID].xyz,1.0);
    // result.position = vec4(pos,1.0);
    result.uv = position.xy;
    result.color = color;

    return result;
}

vec4 PSMain(PSInput input) : SV_TARGET
{
    return SampleTexture(color, defaultSampler, input.uv) + input.color + Color0;
}
