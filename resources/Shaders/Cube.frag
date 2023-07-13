#version 450

#include "Common/Macros.glsl"

layout(location = 0) out vec4 outputColor; 

layout (location = 0) in vec2 FragUV;

struct uniformData
{
    vec4 _Color0;
    vec4 _Color1;
};

DECLARE_UNIFORM_BUFFER(0, 0, UniformData)
{
    uniformData Data;
};

DECLARE_UNIFORM_TEXTURE(0, 4, Texture);


void main() 
{
    outputColor = vec4(FragUV,0,0) + Data._Color0 + texture(Texture, FragUV);
}