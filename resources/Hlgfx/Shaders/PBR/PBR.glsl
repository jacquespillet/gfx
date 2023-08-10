#version 450

#include "../Common/Macros.glsl"
#include "../Common/Bindings.h"

struct PSInput
{
    vec2 FragUV;
    vec3 FragPosition;
    vec3 FragNormal;
    mat3 TBN;
};

DECLARE_UNIFORM_BUFFER(CameraDescriptorSetBinding, CameraBinding, Camera)
{
    float FOV;
    float AspectRatio;
    float NearClip;
    float FarClip;

    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
    mat4 ViewProjectionMatrix;    

    vec4 CameraPosition;
};

DECLARE_UNIFORM_BUFFER(ModelDescriptorSetBinding, ModelBinding, Model)
{
    mat4 ModelMatrix;    
    mat4 NormalMatrix;    
};

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
DECLARE_UNIFORM_BUFFER(MaterialDescriptorSetBinding, MaterialDataBinding, Mat)
{
    materialData Material;
};

DECLARE_UNIFORM_TEXTURE(MaterialDescriptorSetBinding, BaseColorTextureBinding, BaseColorTexture);
DECLARE_UNIFORM_TEXTURE(MaterialDescriptorSetBinding, NormalTextureBinding, NormalTexture);
DECLARE_UNIFORM_TEXTURE(MaterialDescriptorSetBinding, MetallicRoughnessTextureBinding, MetallicRoughnessTexture);
DECLARE_UNIFORM_TEXTURE(MaterialDescriptorSetBinding, OcclusionTextureBinding, OcclusionTexture);
DECLARE_UNIFORM_TEXTURE(MaterialDescriptorSetBinding, EmissiveTextureBinding, EmissionTexture);


/////////////////////////////////
//////////VERTEX/////////////////
/////////////////////////////////
#if defined(VERTEX)
layout(location = 0) in vec4 PositionUvX;
layout(location = 1) in vec4 NormalUvY;
layout(location = 2) in vec4 Tangent;

layout (location = 0) out PSInput Output;

void main() 
{
    // vec4 OutPosition = ViewProjectionMatrix * ModelMatrix *vec4(PositionUvX.xyz, 1.0); 
    // Output.FragUV = vec2(PositionUvX.w, NormalUvY.w);
    // Output.Tangent = Tangent.xyz;

    /////////
	mat4 ModelViewProjection = ViewProjectionMatrix * ModelMatrix;
	vec4 OutPosition = ModelViewProjection * vec4(PositionUvX.xyz, 1.0);
	Output.FragPosition = (ModelMatrix * vec4(PositionUvX.xyz, 1.0)).xyz;
	Output.FragUV = vec2(PositionUvX.w, NormalUvY.w);


	Output.FragNormal = normalize((NormalMatrix * vec4(NormalUvY.xyz, 0.0)).xyz);
    vec3 FragTangent = normalize((NormalMatrix * vec4(Tangent.xyz, 0.0)).xyz);  
    vec3 FragBitangent = normalize(cross(Output.FragNormal, FragTangent.xyz) * Tangent.w); 
	Output.TBN = mat3(FragTangent, FragBitangent, Output.FragNormal);  

    gl_Position = OutPosition;
}

#endif


/////////////////////////////////
//////////FRAGMENT///////////////
/////////////////////////////////
#if defined(FRAGMENT)

layout (location = 0) in PSInput Input;
layout(location = 0) out vec4 OutputColor; 

#include "Util.glsl"
#include "Material.glsl"
#include "Tonemapping.glsl"
#include "BRDF.glsl"

vec3 LightDirection = vec3(-1,-1,-1);
float LightIntensity = 4;

void main() 
{
    //Color
    vec4 BaseColor = GetBaseColor();

    //Normal
    vec3 View = normalize(CameraPosition.xyz - Input.FragPosition);
    normalInfo NormalInfo = GetNormalInfo();
    vec3 Normal = NormalInfo.ShadingNormal;
    vec3 Tangent = NormalInfo.Tangent;
    vec3 Bitangent = NormalInfo.Bitangent;

    float NdotV = ClampedDot(Normal, View);
    float TdotV = ClampedDot(Tangent, View);
    float BdotV = ClampedDot(Bitangent, View);

    materialInfo MaterialInfo;
    MaterialInfo.BaseColor = BaseColor.rgb;
    MaterialInfo.ior = 1.5;
    MaterialInfo.f0 = vec3(0.04);
    MaterialInfo.SpecularWeight = 1.0;
    MaterialInfo = GetMetallicRoughnessInfo(MaterialInfo);

    float Reflectance = max(max(MaterialInfo.f0.r, MaterialInfo.f0.g), MaterialInfo.f0.b);

    MaterialInfo.F90 = vec3(1.0);
    vec3 FinalSpecular = vec3(0.0);
    vec3 FinalDiffuse = vec3(0.0);
    vec3 FinalEmissive = vec3(0.0);

    //Lighting
    vec3 H = normalize(-LightDirection + View);
    float NdotL = ClampedDot(Normal, -LightDirection);
    float VdotH = ClampedDot(View, H);
    float NdotH = ClampedDot(Normal, H);
    FinalDiffuse += LightIntensity * NdotL *  GetBRDFLambertian(MaterialInfo.f0, MaterialInfo.F90, MaterialInfo.CDiff, MaterialInfo.SpecularWeight, VdotH);
    FinalSpecular += LightIntensity * NdotL * GetBRDFSpecularGGX(MaterialInfo.f0, MaterialInfo.F90, MaterialInfo.AlphaRoughness, MaterialInfo.SpecularWeight, VdotH, NdotL, NdotV, NdotH);


    //AO
    float AmbientOcclusion = 1.0;
    AmbientOcclusion = texture(OcclusionTexture, Input.FragUV).r;
    AmbientOcclusion = mix(1, AmbientOcclusion, Material.UseOcclusionTexture);
    FinalDiffuse = mix(FinalDiffuse, FinalDiffuse * AmbientOcclusion, Material.OcclusionStrength);
    FinalSpecular = mix(FinalSpecular, FinalSpecular * AmbientOcclusion, Material.OcclusionStrength);

    //Emissive
    FinalEmissive = Material.Emission * Material.EmissiveFactor;
    vec3 EmissiveSample = texture(EmissionTexture, Input.FragUV).rgb;
    EmissiveSample = mix(vec3(1), EmissiveSample, Material.UseEmissionTexture);
    FinalEmissive *= EmissiveSample;

    vec3 FinalColor = vec3(0);
    FinalColor = FinalEmissive + FinalDiffuse + FinalSpecular;

    if(BaseColor.a < Material.AlphaCutoff)
        discard;
    

    OutputColor = vec4(Tonemap(FinalColor, 1), BaseColor.a);
}

#endif