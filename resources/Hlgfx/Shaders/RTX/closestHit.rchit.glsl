#version 460
#extension GL_EXT_ray_tracing : enable
#extension GL_EXT_nonuniform_qualifier : enable

layout(location = 0) rayPayloadInEXT vec3 hitValue;

struct vertex
{
  vec4 PositionUvX;
  vec4 NormalUvY; 
  vec4 Tangent; 
};
layout (set = 5, binding = 20, std430) buffer VerticesBuffer
{
    vertex Vertices[];
};

layout (set = 5, binding = 21, std430) buffer OffsetsBuffer
{
    uint Offsets[];
};

layout (set = 5, binding = 22, std430) buffer IndexBuffer
{
    uint Indices[];
};

layout (set = 5, binding = 23, std430) buffer MaterialIndicesBuffer
{
    uint MaterialIndices[];
};

layout(set = 6, binding = 0) uniform sampler2D[] Textures;

struct materialData
{
    float RoughnessFactor;
    float MetallicFactor;
    float EmissiveFactor;
    float AlphaCutoff;
    
    vec3 BaseColorFactor;
    float OpacityFactor;

    vec3 Emission;
    float OcclusionStrength;

    float DebugChannel;  
    float UseBaseColor;  
    float UseEmissionTexture;  
    float UseOcclusionTexture;  

    float UseNormalTexture;
    float UseMetallicRoughnessTexture;
    vec2 padding;
};

layout(set = 6, binding = 1) uniform UBO {
    float RoughnessFactor;
    float MetallicFactor;
    float EmissiveFactor;
    float AlphaCutoff;
    
    vec3 BaseColorFactor;
    float OpacityFactor;

    vec3 Emission;
    float OcclusionStrength;

    float DebugChannel;  
    float UseBaseColor;  
    float UseEmissionTexture;  
    float UseOcclusionTexture;  

    float UseNormalTexture;
    float UseMetallicRoughnessTexture;
    float ColourTextureID;
    float padding;
} Materials[];

hitAttributeEXT vec2 attribs;

void main()
{
    // TODO: Shade

    uint Offset = Offsets[gl_InstanceCustomIndexEXT];

    uint Primitive = gl_PrimitiveID * 3;
    uint Inx0 = Indices[Offset + Primitive + 0];
    uint Inx1 = Indices[Offset + Primitive + 1];
    uint Inx2 = Indices[Offset + Primitive + 2];

    vertex V0 = Vertices[Inx0];
    vertex V1 = Vertices[Inx1];
    vertex V2 = Vertices[Inx2];

    vec2 UV0 = vec2(V0.PositionUvX.w, V0.NormalUvY.w);
    vec2 UV1 = vec2(V1.PositionUvX.w, V1.NormalUvY.w);
    vec2 UV2 = vec2(V2.PositionUvX.w, V2.NormalUvY.w);

    uint MaterialIndex = MaterialIndices[gl_InstanceID];
    uint TextureIndex = uint(Materials[MaterialIndex].ColourTextureID);


    const vec3 Barycentric = vec3(1.0f - attribs.x - attribs.y, attribs.x, attribs.y);
    vec2 UV = Barycentric.x * UV0
                + Barycentric.y * UV1
                + Barycentric.z * UV2;

    vec4 Colour = texture(Textures[TextureIndex], UV);

    // 

    // vec3 Normal = Barycentric.x * V0.NormalUvY.xyz
    //               + Barycentric.y * V1.NormalUvY.xyz
    //               + Barycentric.z * V2.NormalUvY.xyz;
    //               + Barycentric.z * V2.NormalUvY.xyz;

    // vec3 Normal = V0.NormalUvY.xyz;
    // hitValue = vec3(UV, 0);
    hitValue = Colour.xyz;
    // hitValue = Normal;
}
