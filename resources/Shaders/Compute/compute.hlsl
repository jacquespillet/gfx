#include "resources/Shaders/Common/Macros.hlsl"


cbuffer MainConstantBuffer1 : register(b0)
{
    vec4 Color0;
    vec4 Color1;
};

RWStructuredBuffer<vec4> InstancePositions : register(u2);

[numthreads(1024, 1, 1)]
void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    InstancePositions[dispatchThreadId.x].y += (Color0.r - 0.5f) * 0.01f;
}