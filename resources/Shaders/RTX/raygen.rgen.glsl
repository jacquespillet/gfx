#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_GOOGLE_include_directive : require

layout(binding = 0, set = 0) uniform accelerationStructureEXT topLevelAS;
layout(binding = 1, set = 0, rgba8) uniform image2D outputImage;

layout(location = 0) rayPayloadEXT vec3 hitValue;
void main() 
{
    vec3 Origin = vec3(0,0,-5);

    const vec2 pixelCenter = vec2(gl_LaunchIDEXT.xy) + vec2(0.5);
    const vec2 inUV = (pixelCenter/vec2(gl_LaunchSizeEXT.xy)) * 2.0 - 1.0;
    // vec2 d = inUV * 2.0 - 1.0;
    vec3 target = vec3(inUV, 0.0);
    // vec2 d = vec2(0,0);
    vec3 Direction = normalize(target - Origin);
    // vec3 Direction = normalize(vec3(d, 1));
    vec3 Radiance = vec3(0,0,0);
    // uint  flags = gl_RayFlagsTerminateOnFirstHitEXT | gl_RayFlagsOpaqueEXT | gl_RayFlagsSkipClosestHitShaderEXT;
    uint  flags = gl_RayFlagsOpaqueEXT;
    traceRayEXT(topLevelAS, flags, 0xff, 1, 2, 0, Origin.xyz, 0.001, Direction.xyz, 10000.0, 0);					    
    Radiance = hitValue;
    // Radiance = vec3(inUV,0/);

    imageStore(outputImage, ivec2(gl_LaunchIDEXT.xy), vec4(Radiance, 1));
}
        