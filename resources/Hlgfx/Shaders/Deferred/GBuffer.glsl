#version 450

#include "../Common/Macros.glsl"
#include "../Common/Bindings.h"

struct PSInput
{
    vec2 FragUV;
    vec3 FragPosition;
    vec3 FragNormal;
    vec3 T;
    vec3 B;
    vec3 N;
    vec4 DepthMapUV[MaxLights];
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

    
    vec4 LeftRightBottomTop;
    vec4 BackFront;
};

struct lightData
{
    vec4 ColorAndIntensity;
    vec4 SizeAndType;
    vec4 Position;
    vec4 Direction;
    mat4 LightSpaceMatrix;
};

DECLARE_UNIFORM_BUFFER(SceneDescriptorSetBinding, SceneBinding, Scene)
{

    float LightCount;
    float MaxLightsCount;
    float ShadowMapSize;
    float ShadowBias;    
    lightData Lights[MaxLights];    
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

// DECLARE_UNIFORM_TEXTURE_SHADOW(SceneDescriptorSetBinding, ShadowMapsBinding, ShadowMap);
DECLARE_UNIFORM_TEXTURE_ARRAY_SHADOW(SceneDescriptorSetBinding, ShadowMapsBinding, ShadowMap);

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
 	mat4 ModelViewProjection = mul(ViewProjectionMatrix , ModelMatrix);
	vec4 OutPosition = mul(ModelViewProjection, vec4(PositionUvX.xyz, 1.0));
	Output.FragPosition = (mul(ModelMatrix, vec4(PositionUvX.xyz, 1.0))).xyz;
	Output.FragUV = vec2(PositionUvX.w, NormalUvY.w);


	Output.FragNormal = normalize((mul(NormalMatrix, vec4(NormalUvY.xyz, 0.0))).xyz);
    vec3 FragTangent = normalize((mul(NormalMatrix, vec4(Tangent.xyz, 0.0))).xyz);  
    vec3 FragBitangent = normalize(mul(cross(Output.FragNormal, FragTangent.xyz), Tangent.w)); 
	Output.T = FragTangent;
	Output.B = FragBitangent;
	Output.N = Output.FragNormal;

    for(int i=0; i<LightCount; i++)
    {
        Output.DepthMapUV[i] =  Lights[i].LightSpaceMatrix * vec4(Output.FragPosition, 1);
    }
    
    gl_Position = OutPosition;
}

#endif


/////////////////////////////////
//////////FRAGMENT///////////////
/////////////////////////////////
#if defined(FRAGMENT)

layout (location = 0) in PSInput Input;

layout (location = 0) out vec4 outPositionDepth;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out uvec4 outAlbedoMetallicRoughnessOcclusionOcclusionStrength;
layout (location = 3) out vec4 outEmission;

#include "../Common/Util.glsl"
#include "../Common/Material.glsl"

void main() 
{
    // //Color
    vec4 BaseColor = GetBaseColor(Input.FragUV);
    if(BaseColor.a < Material.AlphaCutoff)
        discard;

    // Material
    materialInfo MaterialInfo;
    MaterialInfo.BaseColor = BaseColor.rgb;
    MaterialInfo.ior = 1.5;
    MaterialInfo.f0 = vec3(0.04,0.04,0.04);
    MaterialInfo.SpecularWeight = 1.0;
    MaterialInfo = GetMetallicRoughnessInfo(MaterialInfo, Input.FragUV);

    //Emissive
    vec3 FinalEmissive = Material.Emission * Material.EmissiveFactor;
    vec3 EmissiveSample = SampleTexture(EmissionTexture, DefaultSampler, Input.FragUV).rgb;
    EmissiveSample = mix(vec3(1,1,1), EmissiveSample, Material.UseEmissionTexture);
    FinalEmissive *= EmissiveSample;


    //Normal
    vec3 View = normalize(CameraPosition.xyz - Input.FragPosition);
    normalInfo NormalInfo = GetNormalInfo(Input.T, Input.B, Input.N, Input.FragUV);
    vec3 Normal = NormalInfo.ShadingNormal;

    // AO
    float AmbientOcclusion = 1.0;
    AmbientOcclusion = SampleTexture(OcclusionTexture, DefaultSampler, Input.FragUV).r;
    AmbientOcclusion = mix(1, AmbientOcclusion, Material.UseOcclusionTexture);

	outAlbedoMetallicRoughnessOcclusionOcclusionStrength.r = packHalf2x16(BaseColor.rg); //Packs 2 floats into a uint
	outAlbedoMetallicRoughnessOcclusionOcclusionStrength.g = packHalf2x16(BaseColor.ba);
	outAlbedoMetallicRoughnessOcclusionOcclusionStrength.b = packHalf2x16(vec2(MaterialInfo.Metallic, MaterialInfo.PerceptualRoughness));
	outAlbedoMetallicRoughnessOcclusionOcclusionStrength.a = packHalf2x16(vec2(AmbientOcclusion, Material.OcclusionStrength));        
    
    outPositionDepth = vec4(Input.FragPosition, 0);
    outNormal.xyz = Normal  * 0.5 + 0.5;
    outEmission.xyz = FinalEmissive;
}

#endif