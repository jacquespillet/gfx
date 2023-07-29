#version 450

#include "../Common/Macros.glsl"
struct PSInput
{
    vec2 FragUV;
};

DECLARE_UNIFORM_BUFFER(1, 0, Camera)
{
    float FOV;
    float AspectRatio;
    float NearClip;
    float FarClip;

    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
    mat4 ViewProjectionMatrix;    
};

DECLARE_UNIFORM_BUFFER(2, 1, Model)
{
    mat4 ModelMatrix;    
};

DECLARE_UNIFORM_BUFFER(3, 3, Material)
{
    vec4 BaseColor;
};

DECLARE_UNIFORM_TEXTURE(3, 4, DiffuseTexture);


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
    outputColor =  BaseColor + texture(DiffuseTexture, Input.FragUV);
}

#endif