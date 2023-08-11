#include "resources/Hlgfx/Shaders/Common/Macros.hlsl"
#include "resources/Hlgfx/Shaders/Common/Bindings.h"

struct PSInput
{
    vec4 Position : SV_POSITION;
};

cbuffer Camera : register(b0)
{
    float FOV;
    float AspectRatio;
    float NearClip;
    float FarClip;

    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
    mat4 ViewProjectionMatrix;    
    
    vec4 CameraPosition;
};


cbuffer Model : register(b1)
{
    mat4 ModelMatrix;    
    mat4 NormalMatrix;    
};


PSInput VSMain(vec4 PositionUvX : POSITION0, vec4 NormalUvY : POSITION1, vec4 Tangent : POSITION2)
{
    PSInput Output;
	mat4 ModelViewProjection = mul(ViewProjectionMatrix , ModelMatrix);
	vec4 OutPosition = mul(ModelViewProjection, vec4(PositionUvX.xyz, 1.0));
    Output.Position = OutPosition;
    return Output;
}

vec4 PSMain(PSInput Input) : SV_TARGET
{
    vec4 OutputColor = vec4(1,0,0,0);
    return OutputColor;
}
