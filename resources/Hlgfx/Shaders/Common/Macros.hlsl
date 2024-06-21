// CUSTOM_DEFINES


#define vec2 float2
#define vec3 float3
#define vec4 float4
#define mat4 float4x4 
#define mat3 float3x3 

#define ivec2 int2
#define ivec3 int3
#define ivec4 int4

#define uvec2 uint2
#define uvec3 uint3
#define uvec4 uint4


#define SampleTexture(Texture, Sampler, UV) Texture.Sample(Sampler, UV)
#define SampleShadowTextureArray(Texture, Sampler, UV, Depth) Texture.SampleCmp(Sampler, UV, Depth)

#define mix(A, B, T) lerp(A, B, T)

#define texelFetch(Texture, Coords, Mip) Texture.Load(ivec3(Coords, Mip));

ivec2 textureSize(Texture2D<uvec4> Texture, int i)
{
    uint Width, Height, Depth;
    Texture.GetDimensions(i, Width, Height, Depth);

    return ivec2(Width, Height);
}


vec2 unpackHalf2x16(uint Input)
{
    uint x = Input & 0x0000ffff;
    uint y = (Input >> 16) & 0x0000ffff;

    return vec2(f16tof32(x), f16tof32(y));
}

uint packHalf2x16(vec2 Input)
{
    uint x = f32tof16(Input.x);
    uint y = f32tof16(Input.y); 
    return x | (y << 16);
}
