#define GAMMA 2.2
#define INV_GAMMA (1.0 / GAMMA)

#if (GRAPHICS_API == GL || GRAPHICS_API == VK)
const mat3 ACESInputMat = mat3
(
    0.59719, 0.07600, 0.02840,
    0.35458, 0.90834, 0.13383,
    0.04823, 0.01566, 0.83777
);

const mat3 ACESOutputMat = mat3
(
    1.60475, -0.10208, -0.00327,
    -0.53108,  1.10813, -0.07276,
    -0.07367, -0.00605,  1.07602
);
#endif


// ACES filmic tone map approximation
// see https://github.com/TheRealMJP/BakingLab/blob/master/BakingLab/ACES.hlsl
vec3 RRTAndODTFit(vec3 color)
{
    vec3 a = color * (color + 0.0245786) - 0.000090537;
    vec3 b = color * (0.983729 * color + 0.4329510) + 0.238081;
    return a / b;
}

// ODT_SAT => XYZ => D60_2_D65 => sRGB
// tone mapping 
vec3 toneMapACES_Hill(vec3 color)
{
#if (GRAPHICS_API==D3D11 || GRAPHICS_API == D3D12)
    static mat3 ACESInputMat = mat3
    (
        0.59719, 0.35458, 0.04823,
        0.07600, 0.90834, 0.01566,
        0.02840, 0.13383, 0.83777
    );

    static mat3 ACESOutputMat = mat3
    (
        1.60475, -0.53108,-0.07367,
        -0.10208, 1.10813, -0.00605, 
        -0.00327, -0.07276, 1.07602
    );
#endif

    color = mul(ACESInputMat, color);

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = mul(ACESOutputMat, color);

    // Clamp to [0, 1]
    color = clamp(color, 0.0, 1.0);

    return color;
}

vec3 linearTosRGB(vec3 color)
{
    return pow(color, vec3(INV_GAMMA,INV_GAMMA,INV_GAMMA));
}

vec3 Tonemap(vec3 color, float Exposure)
{
    color *= Exposure;
    color /= 0.6;
    color = toneMapACES_Hill(color);
    return linearTosRGB(color);
}