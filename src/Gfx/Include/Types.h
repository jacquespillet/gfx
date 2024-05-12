#pragma once
#include "stdint.h"
#include <glm/glm.hpp>

#if GFX_API==GFX_VK
#include <vulkan/vulkan.hpp>
#elif GFX_API==GFX_GL
#include <glad/gl.h>
#elif GFX_API==GFX_D3D12
#include <d3d12.h>
#elif GFX_API==GFX_D3D11
#include <d3d11.h>
#endif


namespace gfx  
{

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef float f32;
typedef double f64;
typedef size_t sz;
typedef bool b8;

typedef uint32_t bufferHandle;
typedef uint32_t pipelineHandle;
typedef uint32_t imageHandle;
typedef uint32_t renderPassHandle;
typedef uint32_t framebufferHandle;
typedef uint32_t vertexBufferHandle;
typedef uint32_t shaderStateHandle;

typedef glm::vec2 v2f;
typedef glm::vec3 v3f;
typedef glm::vec4 v4f;
typedef glm::ivec2 v2i;
typedef glm::ivec3 v3i;
typedef glm::ivec4 v4i;
typedef glm::mat4 m4x4;

#define GET_API_DATA(VarName, VarType, Object) std::shared_ptr<VarType> VarName = std::static_pointer_cast<VarType>(Object->ApiData);



#if GFX_API==GFX_VK
typedef uint32_t descriptorSetLayoutHandle;
#endif

static const u32 InvalidHandle = 0xffffffff;


namespace commonConstants
{
    static const u8 MaxVertexAttributes = 16;
    static const u32 MaxVertexStreams = 16;
    static const u8 MaxImageOutputs = 8;
    static const u8 MaxShaderStages = 8;
};

enum class clearBufferType
{
    Color, Depth
};


enum class deviceType
{
    CPU = 0,
    DISCRETE_GPU,
    INTEGRATED_GPU,
    VIRTUAL_GPU,
    OTHER
};

struct imageUsage;


enum class format : u32
{
    UNDEFINED = 0,
    R4G4_UNORM_PACK_8,
    R4G4B4A4_UNORM_PACK_16,
    B4G4R4A4_UNORM_PACK_16,
    R5G6B5_UNORM_PACK_16,
    B5G6R5_UNORM_PACK_16,
    R5G5B5A1_UNORM_PACK_16,
    B5G5R5A1_UNORM_PACK_16,
    A1R5G5B5_UNORM_PACK_16,
    R8_UNORM,
    R8_SNORM,
    R8_USCALED,
    R8_SSCALED,
    R8_UINT,
    R8_SINT,
    R8_SRGB,
    R8G8_UNORM,
    R8G8_SNORM,
    R8G8_USCALED,
    R8G8_SSCALED,
    R8G8_UINT,
    R8G8_SINT,
    R8G8_SRGB,
    R8G8B8_UNORM,
    R8G8B8_SNORM,
    R8G8B8_USCALED,
    R8G8B8_SSCALED,
    R8G8B8_UINT,
    R8G8B8_SINT,
    R8G8B8_SRGB,
    B8G8R8_UNORM,
    B8G8R8_SNORM,
    B8G8R8_USCALED,
    B8G8R8_SSCALED,
    B8G8R8_UINT,
    B8G8R8_SINT,
    B8G8R8_SRGB,
    R8G8B8A8_UNORM,
    R8G8B8A8_SNORM,
    R8G8B8A8_USCALED,
    R8G8B8A8_SSCALED,
    R8G8B8A8_UINT,
    R8G8B8A8_SINT,
    R8G8B8A8_SRGB,
    B8G8R8A8_UNORM,
    B8G8R8A8_SNORM,
    B8G8R8A8_USCALED,
    B8G8R8A8_SSCALED,
    B8G8R8A8_UINT,
    B8G8R8A8_SINT,
    B8G8R8A8_SRGB,
    A8B8G8R8_UNORM_PACK_32,
    A8B8G8R8_SNORM_PACK_32,
    A8B8G8R8_USCALED_PACK_32,
    A8B8G8R8_SSCALED_PACK_32,
    A8B8G8R8_UINT_PACK_32,
    A8B8G8R8_SINT_PACK_32,
    A8B8G8R8_SRGB_PACK_32,
    A2R10G10B10_UNORM_PACK_32,
    A2R10G10B10_SNORM_PACK_32,
    A2R10G10B10_USCALED_PACK_32,
    A2R10G10B10_SSCALED_PACK_32,
    A2R10G10B10_UINT_PACK_32,
    A2R10G10B10_SINT_PACK_32,
    A2B10G10R10_UNORM_PACK_32,
    A2B10G10R10_SNORM_PACK_32,
    A2B10G10R10_USCALED_PACK_32,
    A2B10G10R10_SSCALED_PACK_32,
    A2B10G10R10_UINT_PACK_32,
    A2B10G10R10_SINT_PACK_32,
    R16_UNORM,
    R16_SNORM,
    R16_USCALED,
    R16_SSCALED,
    R16_UINT,
    R16_SINT,
    R16_SFLOAT,
    R16G16_UNORM,
    R16G16_SNORM,
    R16G16_USCALED,
    R16G16_SSCALED,
    R16G16_UINT,
    R16G16_SINT,
    R16G16_SFLOAT,
    R16G16B16_UNORM,
    R16G16B16_SNORM,
    R16G16B16_USCALED,
    R16G16B16_SSCALED,
    R16G16B16_UINT,
    R16G16B16_SINT,
    R16G16B16_SFLOAT,
    R16G16B16A16_UNORM,
    R16G16B16A16_SNORM,
    R16G16B16A16_USCALED,
    R16G16B16A16_SSCALED,
    R16G16B16A16_UINT,
    R16G16B16A16_SINT,
    R16G16B16A16_SFLOAT,
    R32_UINT,
    R32_SINT,
    R32_SFLOAT,
    R32G32_UINT,
    R32G32_SINT,
    R32G32_SFLOAT,
    R32G32B32_UINT,
    R32G32B32_SINT,
    R32G32B32_SFLOAT,
    R32G32B32A32_UINT,
    R32G32B32A32_SINT,
    R32G32B32A32_SFLOAT,
    R64_UINT,
    R64_SINT,
    R64_SFLOAT,
    R64G64_UINT,
    R64G64_SINT,
    R64G64_SFLOAT,
    R64G64B64_UINT,
    R64G64B64_SINT,
    R64G64B64_SFLOAT,
    R64G64B64A64_UINT,
    R64G64B64A64_SINT,
    R64G64B64A64_SFLOAT,
    B10G11R11_UFLOAT_PACK_32,
    E5B9G9R9_UFLOAT_PACK_32,
    D16_UNORM,
    X8D24_UNORM_PACK_32,
    D32_SFLOAT,
    S8_UINT,
    D16_UNORM_S8_UINT,
    D24_UNORM_S8_UINT,
    D32_SFLOAT_S8_UINT,
};


enum class type
{
    BYTE,
    UNSIGNED_BYTE,
    SHORT,
    UNSIGNED_SHORT,
    INT,
    UNSIGNED_INT,
    HALF_FLOAT,
    FLOAT,
    DOUBLE,
    FIXED,
    INT_2_10_10_10_REV,
    UNSIGNED_INT_2_10_10_10_REV,
    UNSIGNED_INT_10F_11F_11F_REV,
};

enum class memoryUsage
{
    GpuOnly = 0,
    CpuOnly,
    CpuToGpu,
    GpuToCpu,
    CpuCopy,
    GpuLazilyAllocated
};

enum class stencilOperation
{
    Keep,
    Zero,
    Replace,
    IncrementAndClamp,
    DecrementAndClamp,
    Invert,
    IncrementAndWrap,
    DecrementAndWrap
};

enum class compareOperation
{
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always
};


enum class blendFactor
{
    Zero,
    One,
    SrcColor,
    OneMinusSrcColor,
    DstColor,
    OneMinusDstColor,
    SrcAlpha,
    OneMinusSrcAlpha,
    DstAlpha,
    OneMinusDstAlpha,
    ConstantColor,
    OneMinusConstantColor,
    ConstantAlpha,
    OneMinusConstantAlpha,
    SrcAlphaSaturate,
    Src1Color,
    OneMinusSrc1Color,
    Src1Alpha,
    OneMinusSrc1Alpha
};


enum class blendOperation
{
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max,
    ZeroEXT,
    SrcEXT,
    DstEXT,
    SrcOverEXT,
    DstOverEXT,
    SrcInEXT,
    DstInEXT,
    SrcOutEXT,
    DstOutEXT,
    SrcAtopEXT,
    DstAtopEXT,
    XorEXT,
    MultiplyEXT,
    ScreenEXT,
    OverlayEXT,
    DarkenEXT,
    LightenEXT,
    ColordodgeEXT,
    ColorburnEXT,
    HardlightEXT,
    SoftlightEXT,
    DifferenceEXT,
    ExclusionEXT,
    InvertEXT,
    InvertRgbEXT,
    LineardodgeEXT,
    LinearburnEXT,
    VividlightEXT,
    LinearlightEXT,
    PinlightEXT,
    HardmixEXT,
    HslHueEXT,
    HslSaturationEXT,
    HslColorEXT,
    HslLuminosityEXT,
    PlusEXT,
    PlusClampedEXT,
    PlusClampedAlphaEXT,
    PlusDarkerEXT,
    MinusEXT,
    MinusClampedEXT,
    ContrastEXT,
    InvertOvgEXT,
    RedEXT,
    GreenEXT,
    BlueEXT
};

enum class frontFace
{
    CounterClockwise,
    Clockwise
};

enum class imageLayout
{
    Undefined,
    General,
    ColorAttachmentOptimal,
    DepthStencilAttachmentOptimal,
    DepthStencilReadOnlyOptimal,
    ShaderReadOnlyOptimal,
    TransferSrcOptimal,
    TransferDstOptimal,
    Preinitialized,
    DepthReadOnlyStencilAttachmentOptimal,
    DepthAttachmentStencilReadOnlyOptimal,
    DepthAttachmentOptimal,
    DepthReadOnlyOptimal,
    StencilAttachmentOptimal,
    StencilReadOnlyOptimal,
    ReadOnlyOptimal,
    AttachmentOptimal,
    PresentSrcKHR,
    VideoDecodeDstKHR,
    VideoDecodeSrcKHR,
    VideoDecodeDpbKHR,
    SharedPresentKHR,
    ShadingRateOptimalNV,
    FragmentDensityMapOptimalEXT,
    DepthAttachmentOptimalKHR,
    DepthReadOnlyOptimalKHR,
    StencilAttachmentOptimalKHR,
    StencilReadOnlyOptimalKHR,
    ReadOnlyOptimalKHR,
    AttachmentOptimalKHR,
};



struct bufferUsage
{
    using value = u32;
    enum Bits : value
    {
#if GFX_API==GFX_VK
        UNKNOWN = (value)vk::BufferUsageFlags{ },
        TransferSource = (value)vk::BufferUsageFlagBits::eTransferSrc,
        TransferDestination = (value)vk::BufferUsageFlagBits::eTransferDst,
        UniformTexelBuffer = (value)vk::BufferUsageFlagBits::eUniformTexelBuffer,
        StorageTexelBuffer = (value)vk::BufferUsageFlagBits::eStorageTexelBuffer,
        UniformBuffer = (value)vk::BufferUsageFlagBits::eUniformBuffer,
        StorageBuffer = (value)vk::BufferUsageFlagBits::eStorageBuffer,
        IndexBuffer = (value)vk::BufferUsageFlagBits::eIndexBuffer,
        VertexBuffer = (value)vk::BufferUsageFlagBits::eVertexBuffer,
        IndirectBuffer = (value)vk::BufferUsageFlagBits::eIndirectBuffer,
        ShaderDeviceAddress = (value)vk::BufferUsageFlagBits::eShaderDeviceAddress,
        TransformFeedbackBuffer = (value)vk::BufferUsageFlagBits::eTransformFeedbackBufferEXT,
        TransformFeedbackCounterBuffer = (value)vk::BufferUsageFlagBits::eTransformFeedbackCounterBufferEXT,
        ConditionalRendering = (value)vk::BufferUsageFlagBits::eConditionalRenderingEXT,
        AccelerationStructureBuildInputReadonly = (value)vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
        AccelerationStructureStorage = (value)vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
        ShaderBindingTable = (value)vk::BufferUsageFlagBits::eShaderBindingTableKHR,
#elif GFX_API==GFX_GL
        UNKNOWN,
        TransferSource,
        TransferDestination,
        UniformTexelBuffer,
        StorageTexelBuffer,
        UniformBuffer,
        StorageBuffer,
        IndexBuffer,
        VertexBuffer,
        IndirectBuffer,
        ShaderDeviceAddress,
        TransformFeedbackBuffer,
        TransformFeedbackCounterBuffer,
        ConditionalRendering,
        AccelerationStructureBuildInputReadonly,
        AccelerationStructureStorage,
        ShaderBindingTable
#elif GFX_API==GFX_D3D12
        UNKNOWN,
        TransferSource = D3D12_RESOURCE_STATE_COPY_SOURCE,
        TransferDestination = D3D12_RESOURCE_STATE_COPY_DEST,
        UniformTexelBuffer = D3D12_RESOURCE_STATE_COMMON,
        StorageTexelBuffer = D3D12_RESOURCE_STATE_COMMON,
        UniformBuffer = D3D12_RESOURCE_STATE_COMMON,
        StorageBuffer = D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
        IndexBuffer = D3D12_RESOURCE_STATE_INDEX_BUFFER,
        VertexBuffer = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
        IndirectBuffer = D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
        ShaderDeviceAddress = D3D12_RESOURCE_STATE_COMMON,
        TransformFeedbackBuffer = D3D12_RESOURCE_STATE_COMMON,
        TransformFeedbackCounterBuffer = D3D12_RESOURCE_STATE_COMMON,
        ConditionalRendering = D3D12_RESOURCE_STATE_COMMON,
        AccelerationStructureBuildInputReadonly = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        AccelerationStructureStorage = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
        ShaderBindingTable = D3D12_RESOURCE_STATE_COMMON,
#elif GFX_API==GFX_D3D11
        UNKNOWN,
        TransferSource,
        TransferDestination,
        UniformTexelBuffer,
        StorageTexelBuffer,
        UniformBuffer,
        StorageBuffer,
        IndexBuffer,
        VertexBuffer,
        IndirectBuffer,
        ShaderDeviceAddress,
        TransformFeedbackBuffer,
        TransformFeedbackCounterBuffer,
        ConditionalRendering,
        AccelerationStructureBuildInputReadonly,
        AccelerationStructureStorage,
        ShaderBindingTable,
#endif
    };
};

struct shaderStageFlags
{
    using value = u32;
    enum bits : value
    {
#if GFX_API==GFX_VK
        Vertex = (value)vk::ShaderStageFlagBits::eVertex,
        TessellationControl = (value)vk::ShaderStageFlagBits::eTessellationControl,
        TessellationEvaluation = (value)vk::ShaderStageFlagBits::eTessellationEvaluation,
        Geometry = (value)vk::ShaderStageFlagBits::eGeometry,
        Fragment = (value)vk::ShaderStageFlagBits::eFragment,
        Compute = (value)vk::ShaderStageFlagBits::eCompute,
        AllGraphics = (value)vk::ShaderStageFlagBits::eAllGraphics,
        All = (value)vk::ShaderStageFlagBits::eAll,
        RaygenKHR = (value)vk::ShaderStageFlagBits::eRaygenKHR,
        AnyHitKHR = (value)vk::ShaderStageFlagBits::eAnyHitKHR,
        ClosestHitKHR = (value)vk::ShaderStageFlagBits::eClosestHitKHR,
        MissKHR = (value)vk::ShaderStageFlagBits::eMissKHR,
        IntersectionKHR = (value)vk::ShaderStageFlagBits::eIntersectionKHR,
        CallableKHR = (value)vk::ShaderStageFlagBits::eCallableKHR,
        RaygenNV = (value)vk::ShaderStageFlagBits::eRaygenNV,
        AnyHitNV = (value)vk::ShaderStageFlagBits::eAnyHitNV,
        ClosestHitNV = (value)vk::ShaderStageFlagBits::eClosestHitNV,
        MissNV = (value)vk::ShaderStageFlagBits::eMissNV,
        IntersectionNV = (value)vk::ShaderStageFlagBits::eIntersectionNV,
        CallableNV = (value)vk::ShaderStageFlagBits::eCallableNV,
        TaskNV = (value)vk::ShaderStageFlagBits::eTaskNV,
        MeshNV = (value)vk::ShaderStageFlagBits::eMeshNV,
        TaskEXT = (value)vk::ShaderStageFlagBits::eTaskEXT,
        MeshEXT = (value)vk::ShaderStageFlagBits::eMeshEXT,
        SubpassShadingHUAWEI = (value)vk::ShaderStageFlagBits::eSubpassShadingHUAWEI,
        ClusterCullingHUAWEI = (value)vk::ShaderStageFlagBits::eClusterCullingHUAWEI
#elif GFX_API==GFX_GL
        Vertex =  GL_VERTEX_SHADER,
        TessellationControl =  GL_TESS_CONTROL_SHADER,
        TessellationEvaluation =  GL_TESS_EVALUATION_SHADER,
        Geometry = GL_GEOMETRY_SHADER,
        Fragment = GL_FRAGMENT_SHADER,
        Compute = GL_COMPUTE_SHADER,
        AllGraphics,
        All,
        RaygenKHR,
        AnyHitKHR,
        ClosestHitKHR,
        MissKHR,
        IntersectionKHR,
        CallableKHR,
        RaygenNV,
        AnyHitNV,
        ClosestHitNV,
        MissNV,
        IntersectionNV,
        CallableNV,
        TaskNV,
        MeshNV,
        TaskEXT,
        MeshEXT,
        SubpassShadingHUAWEI,
        ClusterCullingHUAWEI
#elif GFX_API==GFX_D3D12
        Vertex,
        TessellationControl,
        TessellationEvaluation,
        Geometry,
        Fragment,
        Compute,
        AllGraphics,
        All,
        RaygenKHR,
        AnyHitKHR,
        ClosestHitKHR,
        MissKHR,
        IntersectionKHR,
        CallableKHR,
        RaygenNV,
        AnyHitNV,
        ClosestHitNV,
        MissNV,
        IntersectionNV,
        CallableNV,
        TaskNV,
        MeshNV,
        TaskEXT,
        MeshEXT,
        SubpassShadingHUAWEI,
        ClusterCullingHUAWEI
#elif GFX_API==GFX_D3D11
        Vertex,
        TessellationControl,
        TessellationEvaluation,
        Geometry,
        Fragment,
        Compute,
        AllGraphics,
        All,
        RaygenKHR,
        AnyHitKHR,
        ClosestHitKHR,
        MissKHR,
        IntersectionKHR,
        CallableKHR,
        RaygenNV,
        AnyHitNV,
        ClosestHitNV,
        MissNV,
        IntersectionNV,
        CallableNV,
        TaskNV,
        MeshNV,
        TaskEXT,
        MeshEXT,
        SubpassShadingHUAWEI,
        ClusterCullingHUAWEI
#endif
    };
};

//Do not use this direct mapping, but a ImagUsageTONative instead.
// will allow to have DEPTH_STENCIL_SHADER_READ option
struct imageUsage
{
    using value = u32;
    enum bits : value
    {
#if GFX_API==GFX_VK
        UNKNOWN = (value) vk::ImageUsageFlagBits(),
        TRANSFER_SOURCE = (value)vk::ImageUsageFlagBits::eTransferSrc,
        TRANSFER_DESTINATION = (value)vk::ImageUsageFlagBits::eTransferDst,
        SHADER_READ = (value)vk::ImageUsageFlagBits::eSampled,
        STORAGE = (value)vk::ImageUsageFlagBits::eStorage,
        COLOR_ATTACHMENT = (value)vk::ImageUsageFlagBits::eColorAttachment,
        DEPTH_STENCIL_ATTACHMENT = (value)vk::ImageUsageFlagBits::eDepthStencilAttachment,
        INPUT_ATTACHMENT = (value)vk::ImageUsageFlagBits::eInputAttachment,
        FRAGNENT_SHADING_RATE_ATTACHMENT = (value)vk::ImageUsageFlagBits::eFragmentShadingRateAttachmentKHR,
#elif GFX_API==GFX_GL
        UNKNOWN,
        TRANSFER_SOURCE,
        TRANSFER_DESTINATION,
        SHADER_READ,
        STORAGE,
        COLOR_ATTACHMENT,
        DEPTH_STENCIL_ATTACHMENT,
        INPUT_ATTACHMENT,
        FRAGNENT_SHADING_RATE_ATTACHMENT,
#elif GFX_API==GFX_D3D12
        UNKNOWN,
        TRANSFER_SOURCE,
        TRANSFER_DESTINATION,
        SHADER_READ,
        STORAGE,
        COLOR_ATTACHMENT,
        DEPTH_STENCIL_ATTACHMENT,
        INPUT_ATTACHMENT,
        FRAGNENT_SHADING_RATE_ATTACHMENT,
#elif GFX_API==GFX_D3D11
        UNKNOWN,
        TRANSFER_SOURCE,
        TRANSFER_DESTINATION,
        SHADER_READ,
        STORAGE,
        COLOR_ATTACHMENT,
        DEPTH_STENCIL_ATTACHMENT,
        INPUT_ATTACHMENT,
        FRAGNENT_SHADING_RATE_ATTACHMENT,
#endif
    };
};

struct cullMode
{
    using value = u32;
    enum bits : value
    {
#if GFX_API==GFX_VK
        None = (value) vk::CullModeFlagBits::eNone,
        Front = (value) vk::CullModeFlagBits::eFront,
        Back = (value) vk::CullModeFlagBits::eBack,
        FrontAndBack = (value) vk::CullModeFlagBits::eFrontAndBack
#elif GFX_API==GFX_GL
        None = 0,
        Front = GL_FRONT,
        Back = GL_BACK,
        FrontAndBack = GL_FRONT_AND_BACK
#elif GFX_API==GFX_D3D12
        None = D3D12_CULL_MODE_NONE,
        Front = D3D12_CULL_MODE_FRONT,
        Back = D3D12_CULL_MODE_BACK,
        FrontAndBack = D3D12_CULL_MODE_BACK | D3D12_CULL_MODE_FRONT
#elif GFX_API==GFX_D3D11
        None = D3D11_CULL_NONE,
        Front = D3D11_CULL_FRONT,
        Back = D3D11_CULL_BACK,
        FrontAndBack = D3D11_CULL_BACK | D3D11_CULL_FRONT
#endif
    };
};

struct extent2D
{
    u32 Width;
    u32 Height;
};

enum class samplerFilter 
{
    Nearest,
    Linear,
    NearestMipmapNearest,
    LinearMipmapNearest,
    NearestMipmapLinear,
    LinearMipmapLinear
};

//
enum class samplerWrapMode 
{
    ClampToEdge,
    ClampToBorder,
    MirroredRepeat,
    Repeat,
    MirrorClampToEdge
};


enum class vertexInputRate
{
    PerVertex, PerInstance, Count
};

enum class fillMode
{
    WireFrame, 
    Solid, 
    Point, 
    Count
};

enum class indexType
{
    Uint16,Uint32
};


}