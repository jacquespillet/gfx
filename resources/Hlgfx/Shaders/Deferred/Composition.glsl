#version 450

#include "../Common/Macros.glsl"
#include "../Common/Bindings.h"


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



struct PSInput
{
    vec2 FragUV;
};


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
    Output.FragUV = vec2(PositionUvX.w, NormalUvY.w);
#if GRAPHICS_API == VK    
    Output.FragUV.y = 1 - Output.FragUV.y;
#endif
    gl_Position = vec4(PositionUvX.xyz, 1);
}

#endif


/////////////////////////////////
//////////FRAGMENT///////////////
/////////////////////////////////
#if defined(FRAGMENT)

layout (location = 0) in PSInput Input;
layout(location = 0) out vec4 OutputColor; 

DECLARE_UNIFORM_TEXTURE(GBufferDescriptorSetBinding, GBufferPositionBinding, SamplerPositionDepth);
DECLARE_UNIFORM_TEXTURE(GBufferDescriptorSetBinding, GBufferNormalBinding, SamplerNormal);
DECLARE_UNIFORM_TEXTURE_UINT(GBufferDescriptorSetBinding, GBufferAlbedoBinding, samplerAlbedoMetallicRoughnessOcclusionOcclusionStrength);
DECLARE_UNIFORM_TEXTURE(GBufferDescriptorSetBinding, GBufferEmissionBinding, samplerEmission);
DECLARE_UNIFORM_TEXTURE(GBufferDescriptorSetBinding, GBufferReflectionBinding, samplerReflections);



DECLARE_UNIFORM_TEXTURE_ARRAY_SHADOW(SceneDescriptorSetBinding, ShadowMapsBinding, ShadowMap);

// layout(binding = GBufferReflectionBinding, set = GBufferDescriptorSetBinding, rgba8) uniform readonly image2D ReflectionImage;


#define DEFERRED
#include "../Common/Util.glsl"  
#include "../Common/Material.glsl"
#include "../Common/Tonemapping.glsl"
#include "../Common/BRDF.glsl"
void main() 
{
    vec2 inUV = Input.FragUV;

	vec4 PositionDepth = texture(SamplerPositionDepth, inUV);
	vec3 Position = PositionDepth.xyz;
	float Depth = PositionDepth.w;

    if(Depth == 0.0f) discard;

	vec3 Normal = texture(SamplerNormal, inUV).xyz * 2.0 - 1.0;


	ivec2 texDim = textureSize(samplerAlbedoMetallicRoughnessOcclusionOcclusionStrength, 0);
    ivec2 TexelPos = ivec2(inUV.st * texDim);
	uvec4 albedo = texelFetch(samplerAlbedoMetallicRoughnessOcclusionOcclusionStrength, TexelPos, 0);
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

	
	vec3 Emission = texture(samplerEmission, inUV).xyz;

	materialInfo MaterialInfo;
    MaterialInfo.BaseColor = BaseColor.rgb;
    
    
    MaterialInfo.ior = 1.5;
    MaterialInfo.f0 = vec3(0.04);
    MaterialInfo.SpecularWeight = 1.0;
    MaterialInfo = GetMetallicRoughnessInfo(MaterialInfo, Roughness, Metallic);

    vec3 View = normalize(CameraPosition.xyz - Position);
	float NdotV = ClampedDot(Normal, View);

	float Reflectance = max(max(MaterialInfo.f0.r, MaterialInfo.f0.g), MaterialInfo.f0.b);

    MaterialInfo.F90 = vec3(1.0);

    vec3 FinalSpecular = vec3(0.0);
    vec3 FinalDiffuse = vec3(0.0);
    vec3 FinalEmissive = vec3(0.0);
    vec3 FinalClearcoat = vec3(0.0);
    vec3 FinalSheen = vec3(0.0);
    vec3 FinalTransmission = vec3(0.0);

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

            vec4 DepthMapUV =  Lights[i].LightSpaceMatrix * vec4(Position.xyz, 1);
            vec3 ProjCoords = DepthMapUV.xyz / DepthMapUV.w; // Between 0 and 1
#if GRAPHICS_API == VK
            ProjCoords.xy = ProjCoords.xy * 0.5 + 0.5; 
#else
            ProjCoords = ProjCoords * 0.5 + 0.5;
#endif
            
            // float Bias = CalculateBias(Normal, -LightDirection, CurrentDepth);
            float Bias = max(ShadowBias * (1.0 - dot(Normal, -LightDirection)), 0.0001);
            if(ProjCoords.z > 1.0)
                Visibility *= 1.0;
            else 
                Visibility *= mix(0.1f, 1.0f, SampleTexture(ShadowMap, PointWrapSampler, vec4(ProjCoords.xy, i, ProjCoords.z - Bias)).r);
        }
    }

        


    FinalEmissive = Emission;
    
    float MipCount = floor(log2(max(texDim.x, texDim.y))) + 1; 
    vec3 FresnelTerm = FresnelShlick(MaterialInfo.f0, MaterialInfo.F90, NdotV);
    vec3 ReflectionColour = MaterialInfo.PerceptualRoughness < 0.8 ? textureLod(samplerReflections, inUV, (MaterialInfo.PerceptualRoughness * MipCount)).rgb : vec3(0);

    vec3 Color = vec3(0);
    Color = FinalEmissive + FinalDiffuse + FinalSpecular + ReflectionColour * FresnelTerm;
    Color *= Visibility;    
    OutputColor = vec4(Tonemap(Color, 1), BaseColor.a);	
}

#endif