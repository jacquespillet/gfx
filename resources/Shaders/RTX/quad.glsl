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
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 UV;

layout (location = 0) out PSInput Output;

void main() 
{
    gl_Position = vec4(position, 1.0);
    Output.FragUV = UV;
}

#endif


/////////////////////////////////
//////////FRAGMENT///////////////
/////////////////////////////////
#if defined(FRAGMENT)

layout (location = 0) in PSInput Input;
layout(location = 0) out vec4 outputColor; 

layout(binding = 0, set = 0, rgba8) readonly uniform image2D outputImage;

void main() 
{
    ivec2 ImageSize = ivec2(1280, 720);
    ivec2 TexCoord = ivec2(Input.FragUV * vec2(ImageSize));

    vec4 RaytracedColour = imageLoad(outputImage, TexCoord);
    // outputColor = vec4(Input.FragUV, 0, 1) + RaytracedColour;
    outputColor = RaytracedColour;
}

#endif