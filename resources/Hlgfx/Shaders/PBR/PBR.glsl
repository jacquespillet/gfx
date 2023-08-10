#version 450

#include "../Common/Macros.glsl"
#include "../Common/Bindings.h"

struct PSInput
{
    vec2 FragUV;
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
};

DECLARE_UNIFORM_BUFFER(ModelDescriptorSetBinding, ModelBinding, Model)
{
    mat4 ModelMatrix;    
};

DECLARE_UNIFORM_BUFFER(MaterialDescriptorSetBinding, MaterialDataBinding, Material)
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
};

DECLARE_UNIFORM_TEXTURE(MaterialDescriptorSetBinding, BaseColorTextureBinding, BaseColorTexture);
DECLARE_UNIFORM_TEXTURE(MaterialDescriptorSetBinding, OcclusionTextureBinding, OcclusionTexture);
DECLARE_UNIFORM_TEXTURE(MaterialDescriptorSetBinding, EmissiveTextureBinding, EmissionTexture);


/////////////////////////////////
//////////VERTEX/////////////////
/////////////////////////////////
#if defined(VERTEX)
layout(location = 0) in vec4 PositionUvX;
layout(location = 1) in vec4 NormalUvY;

layout (location = 0) out PSInput Output;

void main() 
{
    gl_Position = ViewProjectionMatrix * ModelMatrix *vec4(PositionUvX.xyz, 1.0);
    Output.FragUV = vec2(PositionUvX.w, NormalUvY.w);
}

#endif


/////////////////////////////////
//////////FRAGMENT///////////////
/////////////////////////////////
#if defined(FRAGMENT)

layout (location = 0) in PSInput Input;
layout(location = 0) out vec4 OutputColor; 

void main() 
{
    vec4 FinalColor = vec4(0,0,0,0);

    vec3 FinalEmission = vec3(0,0,0);
    FinalEmission += SampleTexture(EmissionTexture, DefaultSampler, Input.FragUV).rgb;
    FinalEmission = mix(vec3(0,0,0), FinalEmission, UseEmissionTexture);
    FinalEmission *= Emission;
    FinalEmission *= EmissiveFactor;


    float Occlusion = SampleTexture(OcclusionTexture, DefaultSampler, Input.FragUV).x;
    Occlusion = mix(1, Occlusion, UseOcclusionTexture);
    Occlusion = mix(1, Occlusion, OcclusionStrength);

    vec4 BaseColor = SampleTexture(BaseColorTexture, DefaultSampler, Input.FragUV);
    BaseColor = mix(vec4(0,0,0,0), BaseColor, UseBaseColor);

    FinalColor.rgb = BaseColor.rgb * BaseColorFactor.rgb * Occlusion;
    FinalColor.rgb += FinalEmission;
    FinalColor.a = OpacityFactor * BaseColor.a;
    
    OutputColor = FinalColor;
}

#endif