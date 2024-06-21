#define vec2 float2;
#define vec3 float3;
#define vec4 float4;
#define ivec2 int2;
#define ivec3 int3;
#define ivec4 int4;
#define uvec2 uint2;
#define uvec3 uint3;
#define uvec4 uint4;
#define mat3 float3x3;

#define SampleTexture(Texture, Sampler, UV) Texture.Sample(Sampler, UV)

#define texelFetch(Texture, Coords) Texture.Load(Coords);
// #define textureSize(Texture) 
ivec2 textureSize(Texture2D Texture, int i)
{
    uint Width, Height, Depth;
    Texture.GetDimension(Width, Height, Depth);

    return ivec2(Width, Height);
}

vec2 unpackHalf2x16(uint Input)
{
    f16tof32(Input);
}

// #if GRAPHICS_API==D3D11
// #define DECLARE_STORAGE_BUFFER(Set, Binding, Name) layout (set = Set, binding = Binding, std430) buffer Name
// #elif GRAPHICS_API==D3D12
// #define DECLARE_STORAGE_BUFFER(Set, Binding, Name) layout (std430, binding = Binding) buffer Name 
// #endif
#define DECLARE_STORAGE_BUFFER(Set, Binding, Name, Type) StructuredBuffer<Type> Name : register(t##Binding)
