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
    vec4 DepthMapUV;
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
    vec4 LightCount;
    lightData Lights[3];    
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

// DECLARE_UNIFORM_TEXTURE_SHADOW(SceneDescriptorSetBinding, ShadowMapsBindingStart, ShadowMap);
DECLARE_UNIFORM_TEXTURE_ARRAY(SceneDescriptorSetBinding, ShadowMapsBindingStart, ShadowMap);

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

	Output.DepthMapUV = Lights[0].LightSpaceMatrix * vec4(Output.FragPosition, 1);
    // Output.DepthMapUV /= Output.DepthMapUV.w;
	// Output.DepthMapUV.xyz = Output.DepthMapUV.xyz * 0.5 + 0.5;
    
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

void main() 
{
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

    float Visibility = 1.0f;
    //Lighting
    for(int i=0; i<LightCount.x; i++)
    {
        if(Lights[i].SizeAndType.w == PointLight)
        {
            vec3 LightDirection = -normalize(Lights[i].Position.xyz - Input.FragPosition);
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
            vec3 LightDirection = -normalize(Lights[i].Direction.xyz);
            vec3 LightIntensity = Lights[i].ColorAndIntensity.w * Lights[i].ColorAndIntensity.xyz;

            vec3 H = normalize(-LightDirection + View);
            float NdotL = ClampedDot(Normal, -LightDirection);
            float VdotH = ClampedDot(View, H);
            float NdotH = ClampedDot(Normal, H);
            FinalDiffuse += LightIntensity * NdotL *  GetBRDFLambertian(MaterialInfo.f0, MaterialInfo.F90, MaterialInfo.CDiff, MaterialInfo.SpecularWeight, VdotH);
            FinalSpecular += LightIntensity * NdotL * GetBRDFSpecularGGX(MaterialInfo.f0, MaterialInfo.F90, MaterialInfo.AlphaRoughness, MaterialInfo.SpecularWeight, VdotH, NdotL, NdotV, NdotH);

            // float Visibility = texture(ShadowMap, vec3(Input.DepthMapUV.xy, (Input.DepthMapUV.z - 0.0005)/Input.DepthMapUV.w));
            float bias = max(0.001 * (1.0 - dot(Normal, -LightDirection)), 0.0001);  
            vec3 ProjCoords = Input.DepthMapUV.xyz / Input.DepthMapUV.w;
            ProjCoords.xy = ProjCoords.xy * 0.5 + 0.5;
            float ClosestDepth = texture(ShadowMap, vec3(ProjCoords.xy, 0)).x;
            float CurrentDepth = ProjCoords.z;
            Visibility = CurrentDepth - bias > ClosestDepth ? 0.5 : 1.0;
        }
    }

    

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
    FinalColor *= Visibility;

    if(BaseColor.a < Material.AlphaCutoff)
        discard;
    

    OutputColor = vec4(Tonemap(FinalColor, 1), BaseColor.a);
}

#endif