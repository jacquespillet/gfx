#define vec2 float2
#define vec3 float3
#define vec4 float4
#define mat4 float4x4 

#define ivec2 int2
#define ivec3 int3
#define ivec4 int4


#define SampleTexture(Texture, Sampler, UV) Texture.Sample(Sampler, UV)

#define mix(A, B, T) lerp(A, B, T)