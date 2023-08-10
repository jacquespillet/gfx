#define vec2 float2
#define vec3 float3
#define vec4 float4
#define mat4 float4x4 

#define SampleTexture(Texture, Sampler, UV) Texture.Sample(Sampler, UV)

#if GRAPHICS_API == D3D11
#define DECLARE_STORAGE_BUFFER(Set, Binding, Name, Type) StructuredBuffer<Type> Name : register(t##Binding)
#else
#define DECLARE_STORAGE_BUFFER(Set, Binding, Name, Type) RWStructuredBuffer<Type> Name : register(u##Binding)
#endif