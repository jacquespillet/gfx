#include "resources/Shaders/Common/Macros.hlsl"

struct PSInput
{
    vec4 position : SV_POSITION;
    vec2 uv : TEXCOORD;
};

PSInput VSMain(vec4 PositionUvX : POSITION0, vec4 NormalUvY : POSITION1)
{
    PSInput result;

    result.uv = vec2(PositionUvX.w, NormalUvY.w);
    result.position = vec4(PositionUvX.xyz, 1);

    return result;
}

vec4 PSMain(PSInput input) : SV_TARGET
{
    return vec4(input.uv, 0, 1);
}
