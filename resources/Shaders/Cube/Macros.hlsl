#define vec2 float2;
#define vec3 float3;
#define vec4 float4;

#define SampleTexture(Texture, Sampler, UV) Texture.Sample(Sampler, UV)

// #if GRAPHICS_API==D3D11
// #define DECLARE_STORAGE_BUFFER(Set, Binding, Name) layout (set = Set, binding = Binding, std430) buffer Name
// #elif GRAPHICS_API==D3D12
// #define DECLARE_STORAGE_BUFFER(Set, Binding, Name) layout (std430, binding = Binding) buffer Name 
// #endif
#define DECLARE_STORAGE_BUFFER(Set, Binding, Name, Type) StructuredBuffer<Type> Name : register(t##Binding)
