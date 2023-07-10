#define VK 0
#define GL 1
#define GRAPHICS_API VK

#if GRAPHICS_API==VK
#define DECLARE_UNIFORM_BUFFER(Set, Binding, Name) layout (set = Set, binding = Binding) uniform Name
#elif GRAPHICS_API==GL
#define DECLARE_UNIFORM_BUFFER(Set, Binding, Name) layout (std430, binding = Binding) buffer Name 
#endif