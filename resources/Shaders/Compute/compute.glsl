#version 450

#include "../Common/Macros.glsl"

DECLARE_STORAGE_BUFFER(0, 2, StorageBuffer)
{
    vec4 InstancePositions[];
};

DECLARE_UNIFORM_BUFFER(0, 0, UniformData)
{
    vec4 _Color0;
    vec4 _Color1;
};

layout(local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

void main() {
    // Get the global index for this invocation
    uint globalIndex = gl_GlobalInvocationID.x;
    InstancePositions[globalIndex].y += (_Color0.r - 0.5f) * 0.01f;
}