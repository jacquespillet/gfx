#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout (location = 0) out vec4 FragColor;


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
    gl_Position = vec4(position, 1.0);
    FragColor = color;
}