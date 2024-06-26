#version 460
#include "../Common/Macros.glsl"
#include "../Common/Bindings.h"

#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

layout(binding = 16, set = 5) uniform accelerationStructureEXT topLevelAS;
layout(binding = 17, set = 5, rgba8) uniform image2D outputImage;
layout(binding = 18, set = 5) uniform sampler2D SamplerPositionDepth;
layout(binding = 19, set = 5) uniform sampler2D SamplerNormal;


DECLARE_UNIFORM_BUFFER(CameraDescriptorSetBinding, CameraBinding, Camera)
{
    float FOV;
    float AspectRatio;
    float NearClip;
    float FarClip;

    mat4 ProjectionMatrix;
    mat4 ViewMatrix;
    mat4 ViewProjectionMatrix;    

    vec4 CameraPosition;

    
    vec4 LeftRightBottomTop;
    vec4 BackFront;
};


layout(location = 0) rayPayloadEXT vec3 hitValue;
void main() 
{

    vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);

    vec2 UV = (pixelCenter/vec2(gl_LaunchSizeEXT.xy));
    vec4 PositionDepth = texture(SamplerPositionDepth, UV);
    vec3 Normal = texture(SamplerNormal, UV).xyz * 2.0 - 1.0;


    vec3 Origin = PositionDepth.xyz;
    float Depth = PositionDepth.w;
    if(Depth == 0.0f) return;

    vec3 EyeDir = normalize(Origin - CameraPosition.xyz);

    vec3 Direction = reflect(EyeDir, Normal);

    vec2 LaunchID = vec2(gl_LaunchIDEXT.xy) / gl_LaunchSizeEXT.xy;
    LaunchID.y = 1.0 - LaunchID.y;

    // Args : AS, RayFlags, CullMask, sbtRecordOffset, SbtRecordStride, MissIndex, rayOrigin, tNear, rayDirection, tFar, payload
    traceRayEXT(topLevelAS, gl_RayFlagsOpaqueEXT, 0xff, 0, 0, 0, Origin.xyz, 0.001, Direction.xyz, 10000.0, 0);					    
    vec3 ReflectedColour = hitValue;
    

    imageStore(outputImage, ivec2(gl_LaunchIDEXT.xy), vec4(ReflectedColour, 1));
}
        