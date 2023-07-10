#version 450

#include "Common/Macros.glsl"

layout(location = 0) out vec4 outputColor; 

layout (location = 0) in vec4 FragColor;

struct uniformData
{
    vec4 _Color0;
    vec4 _Color1;
};

DECLARE_UNIFORM_BUFFER(0, 0, UniformData)
{
    uniformData Data;
};

layout (set=0, binding = 4) uniform sampler2D Texture;



void main() 
{
    outputColor = FragColor + Data._Color0 + texture(Texture, FragColor.xy);
}