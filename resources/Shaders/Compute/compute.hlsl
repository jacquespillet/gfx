#include "resources/Shaders/Common/Macros.hlsl"

[numthreads(16, 16, 1)]
void CSMain(uint3 dispatchThreadId : SV_DispatchThreadID)
{
    uint globalIndex = dispatchThreadId.x + dispatchThreadId.y * DispatchThreadID.z * DispatchThreadID.x;
}