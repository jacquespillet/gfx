#include "resources/Hlgfx/Shaders/Common/Macros.hlsl"
#include "resources/Hlgfx/Shaders/Common/Bindings.h"

struct PSInput
{
    vec4 Position : SV_POSITION;
    
    vec2 FragUV : TEXCOORD;
    vec3 FragPosition : NORMAL0;
    vec3 FragNormal : NORMAL1;
    vec3 T : POSITION2;
    vec3 B : POSITION3;
    vec3 N : POSITION4;
    vec4 DepthMapUV[MaxLights] : POSITION5;
};

cbuffer Camera : register(b0)
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


struct lightData
{
    vec4 ColorAndIntensity;
    vec4 SizeAndType;
    vec4 Position;
    vec4 Direction;
    mat4 LightSpaceMatrix;
};
cbuffer Scene : register(b2)
{
    float LightCount;
    float MaxLightsCount;
    float ShadowMapSize;
    float ShadowBias;
    lightData Lights[MaxLights];
};

cbuffer Model : register(b1)
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
cbuffer Material : register(b3)
{
    materialData Material; 
};

Texture2D BaseColorTexture : register(t4);
Texture2D MetallicRoughnessTexture : register(t5);
Texture2D OcclusionTexture : register(t6);
Texture2D NormalTexture : register(t7);
Texture2D EmissionTexture : register(t8);
Texture2DArray ShadowMap : register(t10);

SamplerState DefaultSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

#include "resources/Hlgfx/Shaders/PBR/Util.glsl"
#include "resources/Hlgfx/Shaders/PBR/Material.glsl"
#include "resources/Hlgfx/Shaders/PBR/Tonemapping.glsl"
#include "resources/Hlgfx/Shaders/PBR/BRDF.glsl"


PSInput VSMain(vec4 PositionUvX : POSITION0, vec4 NormalUvY : POSITION1, vec4 Tangent : POSITION2)
{
    PSInput Output;

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

    Output.Position = OutPosition;
    
    for(int i=0; i<MaxLights; i++)
    {
        Output.DepthMapUV[i] = mul(Lights[i].LightSpaceMatrix, vec4(Output.FragPosition, 1));
    }

    return Output;
}

struct PSOutput {
    vec4 outPositionDepth : SV_Target0;
    vec4 outNormal : SV_Target1;
    uvec4 outAlbedoMetallicRoughnessOcclusionOcclusionStrength : SV_Target2;
    vec4 outEmission : SV_Target3;
};

PSOutput PSMain(PSInput Input)
{
    PSOutput Output;
    
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

	Output.outAlbedoMetallicRoughnessOcclusionOcclusionStrength.r = packHalf2x16(BaseColor.rg); //Packs 2 floats into a uint
	Output.outAlbedoMetallicRoughnessOcclusionOcclusionStrength.g = packHalf2x16(BaseColor.ba);
	Output.outAlbedoMetallicRoughnessOcclusionOcclusionStrength.b = packHalf2x16(vec2(MaterialInfo.Metallic, MaterialInfo.PerceptualRoughness));
	Output.outAlbedoMetallicRoughnessOcclusionOcclusionStrength.a = packHalf2x16(vec2(AmbientOcclusion, Material.OcclusionStrength));        
    
    Output.outPositionDepth = vec4(Input.FragPosition, 0);
    Output.outNormal = vec4(Normal  * 0.5 + 0.5, 0);
    Output.outEmission = vec4(FinalEmissive, 0);

    return Output;
}
