#version 450

#include "Common/Macros.glsl"

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 uv;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout (location = 0) out vec2 FragUV;

struct sceneMatrices
{
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

DECLARE_UNIFORM_BUFFER(0, 5, SceneMatrices)
{
    sceneMatrices Matrices;
};

void main() 
{
    mat4 VP = Matrices.ProjectionMatrix * Matrices.ViewMatrix;
    gl_Position = VP * vec4(position, 1.0);
    FragUV = uv;
}