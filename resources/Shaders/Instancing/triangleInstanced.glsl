#version 450

#include "../Common/Macros.glsl"
struct PSInput
{
    vec4 FragColor;
};


DECLARE_STORAGE_BUFFER(0, 2, StorageBuffer)
{
    vec4 InstancePositions[];
};


/////////////////////////////////
//////////VERTEX/////////////////
/////////////////////////////////
#if defined(VERTEX)

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 instancePosition;

layout (location = 0) out PSInput Output;

void main() 
{
    vec3 pos = position * 0.1;
    gl_Position = vec4(pos +  InstancePositions[InstanceIndex].xyz,1.0);
    Output.FragColor = color;
}
#endif



/////////////////////////////////
//////////FRAGMENT///////////////
/////////////////////////////////
#if defined(FRAGMENT)
layout (location = 0) in PSInput Input;

layout(location = 0) out vec4 outputColor; 


DECLARE_UNIFORM_BUFFER(0, 0, UniformData)
{
    vec4 _Color0;
    vec4 _Color1;
};

DECLARE_UNIFORM_BUFFER(0, 3, UniformData2)
{
    vec4 _Color2;
    vec4 _Color3;
};

DECLARE_UNIFORM_TEXTURE(0, 4, Texture);


void main() 
{
    outputColor = Input.FragColor + _Color0 + _Color2 + texture(Texture, Input.FragColor.xy);
}
#endif

