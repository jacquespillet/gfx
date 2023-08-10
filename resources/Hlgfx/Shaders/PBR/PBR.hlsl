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
SamplerState DefaultSampler : register(s0);

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
    return Output;
}

vec4 PSMain(PSInput Input) : SV_TARGET
{
    vec3 LightDirection = vec3(-1,-1,-1);
    float LightIntensity = 4;

    vec4 OutputColor = vec4(0,0,0,0);

    //Color
    vec4 BaseColor = GetBaseColor(Input.FragUV);

    //Normal
    vec3 View = normalize(CameraPosition.xyz - Input.FragPosition);
    normalInfo NormalInfo = GetNormalInfo(Input.T, Input.B, Input.N, Input.FragUV);
    vec3 Normal = NormalInfo.ShadingNormal;
    vec3 Tangent = NormalInfo.Tangent;
    vec3 Bitangent = NormalInfo.Bitangent;

    float NdotV = ClampedDot(Normal, View);
    float TdotV = ClampedDot(Tangent, View);
    float BdotV = ClampedDot(Bitangent, View);

    materialInfo MaterialInfo;
    MaterialInfo.BaseColor = BaseColor.rgb;
    MaterialInfo.ior = 1.5;
    MaterialInfo.f0 = vec3(0.04,0.04,0.04);
    MaterialInfo.SpecularWeight = 1.0;
    MaterialInfo = GetMetallicRoughnessInfo(MaterialInfo, Input.FragUV);

    float Reflectance = max(max(MaterialInfo.f0.r, MaterialInfo.f0.g), MaterialInfo.f0.b);

    MaterialInfo.F90 = vec3(1.0,1.0,1.0);
    vec3 FinalSpecular = vec3(0.0,0.0,0.0);
    vec3 FinalDiffuse = vec3(0.0,0.0,0.0);
    vec3 FinalEmissive = vec3(0.0,0.0,0.0);

    //Lighting
    vec3 H = normalize(-LightDirection + View);
    float NdotL = ClampedDot(Normal, -LightDirection);
    float VdotH = ClampedDot(View, H);
    float NdotH = ClampedDot(Normal, H);
    FinalDiffuse += LightIntensity * NdotL *  GetBRDFLambertian(MaterialInfo.f0, MaterialInfo.F90, MaterialInfo.CDiff, MaterialInfo.SpecularWeight, VdotH);
    FinalSpecular += LightIntensity * NdotL * GetBRDFSpecularGGX(MaterialInfo.f0, MaterialInfo.F90, MaterialInfo.AlphaRoughness, MaterialInfo.SpecularWeight, VdotH, NdotL, NdotV, NdotH);


    //AO
    float AmbientOcclusion = 1.0;
    AmbientOcclusion = SampleTexture(OcclusionTexture, DefaultSampler, Input.FragUV).r;
    AmbientOcclusion = mix(1, AmbientOcclusion, Material.UseOcclusionTexture);
    FinalDiffuse = mix(FinalDiffuse, FinalDiffuse * AmbientOcclusion, Material.OcclusionStrength);
    FinalSpecular = mix(FinalSpecular, FinalSpecular * AmbientOcclusion, Material.OcclusionStrength);

    //Emissive
    FinalEmissive = Material.Emission * Material.EmissiveFactor;
    vec3 EmissiveSample = SampleTexture(EmissionTexture, DefaultSampler, Input.FragUV).rgb;
    EmissiveSample = mix(vec3(1,1,1), EmissiveSample, Material.UseEmissionTexture);
    FinalEmissive *= EmissiveSample;

    vec3 FinalColor = vec3(0,0,0);
    FinalColor = FinalEmissive + FinalDiffuse + FinalSpecular;

    if(BaseColor.a < Material.AlphaCutoff)
        discard;
    

    OutputColor = vec4(Tonemap(FinalColor, 1), BaseColor.a);
    
    return OutputColor;
}
