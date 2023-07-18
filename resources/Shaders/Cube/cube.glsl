#version 450

#include "../Common/Macros.glsl"
struct PSInput
{
    vec2 FragUV;
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

DECLARE_UNIFORM_TEXTURE(0, 4, Texture);


void main() 
{
    outputColor = vec4(Input.FragUV,0,0) + Data._Color0 + SampleTexture(Texture, Input.FragUV);
}
#endif