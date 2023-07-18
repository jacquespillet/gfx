#define VK 0
#define GL 1
CUSTOM_DEFINES


#if GRAPHICS_API==VK
#define DECLARE_UNIFORM_BUFFER(Set, Binding, Name) layout (set = Set, binding = Binding) uniform Name
#elif GRAPHICS_API==GL
#define DECLARE_UNIFORM_BUFFER(Set, Binding, Name) layout (std430, binding = Binding) buffer Name 
#endif

#if GRAPHICS_API==VK
#define DECLARE_UNIFORM_TEXTURE(Set, Binding, Name) layout(set = Set, binding = Binding) uniform sampler2D Name
#elif GRAPHICS_API==GL
#define DECLARE_UNIFORM_TEXTURE(Set, Binding, Name) layout(binding = Binding) uniform sampler2D Name;
#endif

#define mul(Elem1, Elem2) Elem1 * Elem2

// #define SampleTexture(Texture, Sampler, UV) texture(Texture, UV)
#define SampleTexture(Texture, UV) texture(Texture, UV)