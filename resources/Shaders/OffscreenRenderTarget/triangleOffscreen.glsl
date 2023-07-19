#version 450

#include "../Common/Macros.glsl"
struct PSInput
{
    vec4 FragColor;
};


/////////////////////////////////
//////////VERTEX/////////////////
/////////////////////////////////
#if defined(VERTEX)
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

layout (location = 0) out PSInput Output;

void main() 
{
    gl_Position = vec4(position, 1.0);
    Output.FragColor = color;
}

#endif


/////////////////////////////////
//////////FRAGMENT///////////////
/////////////////////////////////
#if defined(FRAGMENT)

layout (location = 0) in PSInput Input;
layout(location = 0) out vec4 outputColor; 

struct uniformData
{
    vec4 _Color0;
    vec4 _Color1;
};

DECLARE_UNIFORM_TEXTURE(0, 4, Texture);


void main() 
{
    outputColor = texture(Texture, Input.FragColor.xy);
}

#endif