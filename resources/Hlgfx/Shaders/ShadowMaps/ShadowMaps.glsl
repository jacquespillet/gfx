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
};


DECLARE_UNIFORM_BUFFER(ModelDescriptorSetBinding, ModelBinding, Model)
{
    mat4 ModelMatrix;    
    mat4 NormalMatrix;    
};


/////////////////////////////////
//////////VERTEX/////////////////
/////////////////////////////////
#if defined(VERTEX)
layout(location = 0) in vec4 PositionUvX;
layout(location = 1) in vec4 NormalUvY;
layout(location = 2) in vec4 Tangent;


void main() 
{
	mat4 ModelViewProjection = mul(ViewProjectionMatrix , ModelMatrix);
	vec4 OutPosition = mul(ModelViewProjection, vec4(PositionUvX.xyz, 1.0));
    gl_Position = OutPosition;
}

#endif


/////////////////////////////////
//////////FRAGMENT///////////////
/////////////////////////////////
#if defined(FRAGMENT)

layout(location = 0) out vec4 OutputColor; 

void main() 
{
    OutputColor = vec4(1,0,0,0);
}

#endif