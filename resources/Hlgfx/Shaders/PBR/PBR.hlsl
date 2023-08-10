#include "resources/Hlgfx/Shaders/Common/Macros.hlsl"
#include "resources/Hlgfx/Shaders/Common/Bindings.h"

struct PSInput
{
    vec4 position : SV_POSITION;
    vec2 FragUV : TEXCOORD;
    vec3 Tangent : NORMAL;
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
};

cbuffer Model : register(b1)
{
    mat4 ModelMatrix;    
};

cbuffer Material : register(b3)
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
    vec2 Padding0;  
};

Texture2D BaseColorTexture : register(t4);
Texture2D OcclusionTexture : register(t6);
Texture2D EmissionTexture : register(t8);


SamplerState DefaultSampler : register(s0);

PSInput VSMain(vec4 PositionUvX : POSITION0, vec4 NormalUvY : POSITION1, vec4 Tangent : POSITION2)
{
    PSInput Output;

    Output.FragUV = vec2(PositionUvX.w, NormalUvY.w);
    Output.position = mul(mul(ViewProjectionMatrix, ModelMatrix), vec4(PositionUvX.xyz, 1));
    Output.Tangent = Tangent.xyz;
    return Output;
}

vec4 PSMain(PSInput Input) : SV_TARGET
{
    vec4 FinalColor = vec4(0,0,0,0);

    vec3 FinalEmission = vec3(0,0,0);
    FinalEmission += Emission;
    FinalEmission += SampleTexture(EmissionTexture, DefaultSampler, Input.FragUV).rgb;
    FinalEmission *= EmissiveFactor;


    float Occlusion = SampleTexture(OcclusionTexture, DefaultSampler, Input.FragUV).x;
    Occlusion = mix(1, Occlusion, OcclusionStrength);

    vec4 BaseColor = SampleTexture(BaseColorTexture, DefaultSampler, Input.FragUV);
    BaseColor = mix(vec4(0,0,0,0), BaseColor, UseBaseColor);

    FinalColor.rgb = BaseColor.rgb * BaseColorFactor.rgb * Occlusion;
    FinalColor.rgb += FinalEmission;
    FinalColor.a = OpacityFactor * BaseColor.a;
    
    return FinalColor + vec4(Input.Tangent, 0);
}
