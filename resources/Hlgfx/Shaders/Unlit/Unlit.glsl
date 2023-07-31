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

    int DebugChannel;
    ivec3 Padding0;

    float OcclusionStrength;
    vec3 Emission;    
};

DECLARE_UNIFORM_TEXTURE(MaterialDescriptorSetBinding, BaseColorTextureBinding, BaseColorTexture);


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
layout(location = 0) out vec4 outputColor; 

void main() 
{
    outputColor =  texture(BaseColorTexture, Input.FragUV);
}

#endif