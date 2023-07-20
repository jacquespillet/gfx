#version 450

#include "../Common/Macros.glsl"
struct PSInput
{
    vec2 FragUV;
    vec3 position;
};

DECLARE_UNIFORM_BUFFER(0, 5, SceneMatrices)
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

/////////////////////////////////
//////////VERTEX/////////////////
/////////////////////////////////
#if defined(VERTEX)

layout(location = 0) in vec4 position;
layout(location = 1) in vec2 uv;

layout (location = 0) out PSInput Output;


void main() 
{
    mat4 VP = mul(ProjectionMatrix, ViewMatrix);
    gl_Position = mul(VP, position);
    Output.FragUV = uv;
    Output.position = position.xyz;
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
DECLARE_UNIFORM_BUFFER(0, 0, UniformData)
{
    uniformData Data;
};

DECLARE_UNIFORM_CUBEMAP(0, 4, Texture);


void main() 
{
    outputColor = SampleTexture(Texture, Input.position);
}
#endif