#version 450

#include "../Common/Macros.glsl"
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

layout (location = 0) out PSInput Output;

void main() 
{
    gl_Position = vec4(PositionUvX.xyz, 1.0);
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
    outputColor = vec4(Input.FragUV, 0, 1);
}

#endif