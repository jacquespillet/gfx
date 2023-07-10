#define VK 0
#define GL 1
#define GRAPHICS_API GL

#if GRAPHICS_API==VK
#define DECLARE_UNIFORM_BUFFER(Binding, Name, Set) layout (set = Set, binding = Binding) uniform Name
#elif GRAPHICS_API==GL
#define DECLARE_UNIFORM_BUFFER(Binding, Name) layout (std430, binding = Binding) buffer Name 
#endif