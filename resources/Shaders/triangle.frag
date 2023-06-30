#version 450

layout(location = 0) out vec4 outputColor; 

layout (location = 0) in vec4 FragColor;

struct uniformData
{
    vec4 _Color0;
    vec4 _Color1;
};
layout(set = 0, binding = 0) uniform UniformData
{
    uniformData Data;
};

void main() 
{
    outputColor = FragColor + Data._Color0;
}