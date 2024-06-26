#include "resources/Hlgfx/Shaders/Common/Macros.hlsl"
#include "resources/Hlgfx/Shaders/Common/Bindings.h"


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


cbuffer Scene : register(b2)
{

    float LightCount;
    float MaxLightsCount;
    float ShadowMapSize;
    float ShadowBias;    
    lightData Lights[MaxLights];    
};



struct PSInput
{
    vec4 Position : SV_POSITION;
    vec2 FragUV : TEXCOORD;
};


/////////////////////////////////
//////////VERTEX/////////////////
/////////////////////////////////
PSInput VSMain(vec4 PositionUvX : POSITION0, vec4 NormalUvY : POSITION1, vec4 Tangent : POSITION2)
{
    PSInput Output;
    Output.FragUV = vec2(PositionUvX.w, NormalUvY.w);
    Output.Position = vec4(PositionUvX.xyz, 1);
    Output.FragUV.y = 1 - Output.FragUV.y;
    return Output;
}


/////////////////////////////////
//////////FRAGMENT///////////////
/////////////////////////////////

SamplerState DefaultSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);


Texture2DArray ShadowMap : register(t10);

Texture2D SamplerPositionDepth : register(t11);
Texture2D SamplerNormal : register(t12);
Texture2D<uvec4> samplerAlbedoMetallicRoughnessOcclusionOcclusionStrength : register(t13);
Texture2D samplerEmission : register(t14);



#define DEFERRED

#include "resources/Hlgfx/Shaders/Common/Util.glsl"
#include "resources/Hlgfx/Shaders/Common/Material.glsl"
#include "resources/Hlgfx/Shaders/Common/Tonemapping.glsl"
#include "resources/Hlgfx/Shaders/Common/BRDF.glsl"


vec4 PSMain(PSInput Input) : SV_TARGET
{
    vec2 inUV = Input.FragUV;

	vec4 PositionDepth = SampleTexture(SamplerPositionDepth, DefaultSampler, inUV);
	vec3 Position = PositionDepth.xyz;
	float Depth = PositionDepth.w;

	vec3 Normal = SampleTexture(SamplerNormal, DefaultSampler, inUV).xyz * 2.0 - 1.0;

	ivec2 texDim = textureSize(samplerAlbedoMetallicRoughnessOcclusionOcclusionStrength, 0);
	uvec4 albedo = texelFetch(samplerAlbedoMetallicRoughnessOcclusionOcclusionStrength, ivec2(inUV * texDim ), 0);
	vec4 BaseColor;
	BaseColor.rg = unpackHalf2x16(albedo.r);
	BaseColor.ba = unpackHalf2x16(albedo.g);

	vec2 MetallicRoughness;
	MetallicRoughness = unpackHalf2x16(albedo.b);
	float Metallic = MetallicRoughness.x;
	float Roughness = MetallicRoughness.y;

	vec2 OcclusionOcclusionStrength;
	OcclusionOcclusionStrength = unpackHalf2x16(albedo.a);
	float AmbientOcclusion = OcclusionOcclusionStrength.x;
	float OcclusionStrength = OcclusionOcclusionStrength.y;

	
	vec3 Emission = SampleTexture(samplerEmission, DefaultSampler, inUV).xyz;

	materialInfo MaterialInfo;
    MaterialInfo.BaseColor = BaseColor.rgb;
    
    
    MaterialInfo.ior = 1.5;
    MaterialInfo.f0 = vec3(0.04, 0.04, 0.04);
    MaterialInfo.SpecularWeight = 1.0;
    MaterialInfo = GetMetallicRoughnessInfo(MaterialInfo, Roughness, Metallic);

    vec3 View = normalize(CameraPosition.xyz - Position);
	float NdotV = ClampedDot(Normal, View);

	float Reflectance = max(max(MaterialInfo.f0.r, MaterialInfo.f0.g), MaterialInfo.f0.b);

    MaterialInfo.F90 = vec3(1.0, 1.0, 1.0);

    vec3 FinalSpecular = vec3(0.0, 0.0, 0.0);
    vec3 FinalDiffuse = vec3(0.0, 0.0, 0.0);
    vec3 FinalEmissive = vec3(0.0, 0.0, 0.0);
    vec3 FinalClearcoat = vec3(0.0, 0.0, 0.0);
    vec3 FinalSheen = vec3(0.0, 0.0, 0.0);
    vec3 FinalTransmission = vec3(0.0, 0.0, 0.0);

    float Visibility = 1.0f;
    for(int i=0; i<LightCount; i++)
    {
        if(Lights[i].SizeAndType.w == PointLight)
        {
            vec3 LightDirection = -normalize(Lights[i].Position.xyz - Position);
            vec3 LightIntensity = Lights[i].ColorAndIntensity.w * Lights[i].ColorAndIntensity.xyz;

            vec3 H = normalize(-LightDirection + View);
            float NdotL = ClampedDot(Normal, -LightDirection);
            float VdotH = ClampedDot(View, H);
            float NdotH = ClampedDot(Normal, H);
            FinalDiffuse += LightIntensity * NdotL *  GetBRDFLambertian(MaterialInfo.f0, MaterialInfo.F90, MaterialInfo.CDiff, MaterialInfo.SpecularWeight, VdotH);
            FinalSpecular += LightIntensity * NdotL * GetBRDFSpecularGGX(MaterialInfo.f0, MaterialInfo.F90, MaterialInfo.AlphaRoughness, MaterialInfo.SpecularWeight, VdotH, NdotL, NdotV, NdotH);
        }
        else if(Lights[i].SizeAndType.w == DirectionalLight)
        {
            vec3 LightDirection = normalize(Lights[i].Direction.xyz);
            vec3 LightIntensity = Lights[i].ColorAndIntensity.w * Lights[i].ColorAndIntensity.xyz;

            vec3 H = normalize(-LightDirection + View);
            float NdotL = ClampedDot(Normal, -LightDirection);
            float VdotH = ClampedDot(View, H);
            float NdotH = ClampedDot(Normal, H);
            FinalDiffuse += LightIntensity * NdotL *  GetBRDFLambertian(MaterialInfo.f0, MaterialInfo.F90, MaterialInfo.CDiff, MaterialInfo.SpecularWeight, VdotH);
            FinalSpecular += LightIntensity * NdotL * GetBRDFSpecularGGX(MaterialInfo.f0, MaterialInfo.F90, MaterialInfo.AlphaRoughness, MaterialInfo.SpecularWeight, VdotH, NdotL, NdotV, NdotH);

            vec4 DepthMapUV =  mul(Lights[i].LightSpaceMatrix, vec4(Position.xyz, 1));
            vec3 ProjCoords = DepthMapUV.xyz;
            ProjCoords.xy /= DepthMapUV.w;
            ProjCoords.xy = ProjCoords.xy * 0.5 + 0.5;
            ProjCoords.y = 1 - ProjCoords.y;
  
            // float Bias = CalculateBias(Normal, -LightDirection, CurrentDepth);
            float Bias = max(ShadowBias * (1.0 - dot(Normal, -LightDirection)), 0.0001);
            if(ProjCoords.z > 1.0)
                Visibility *= 1.0;
            else 
                Visibility *= mix(0.1f, 1.0f, SampleShadowTextureArray(ShadowMap, ShadowSampler, vec3(ProjCoords.xy, i), ProjCoords.z - Bias).r);
        }
    }

        


    FinalEmissive = Emission;


    vec3 Color = vec3(0,0,0);
    Color = FinalEmissive + FinalDiffuse + FinalSpecular;
    Color *= Visibility;
    
    vec4 OutputColor = vec4(Tonemap(Color, 1), BaseColor.a);	
	return OutputColor;
}